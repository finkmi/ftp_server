#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <libgen.h>

#define BUFFER_SIZE 4096

int quitflag = 0;
int connectflag = 0;

void send_file(char *path, int sockfd) {

	char buffer[BUFFER_SIZE];
	FILE *fp;
	int n;
	struct stat sb;
	
	//Open file pointer
	if((fp = fopen(path, "r")) == NULL) {
		printf("Error opening file pointer\n");
		return;
	}

	//Get file stats
	if (stat(path, &sb) == -1) {
        	printf("Error getting file stats\n");
		return;
    	}

	//Send filesize
	sprintf(buffer, "%lld", sb.st_size);
	if((n = write(sockfd, buffer, strlen(buffer))) < 0) {
		printf("Error writing file size to socket\n");
		return;
	}

	//Send filename
	if((n = write(sockfd, basename(path), strlen(basename(path)))) < 0) {
		printf("Error writing filename to socket\n");
		return;
	}

	//Loop through file sending BUFFER_SIZE bytes, zeroing buffer and repeating until all bytes sent
	while(fgets(buffer, BUFFER_SIZE, fp) != NULL) {

		if((n = write(sockfd, buffer, strlen(buffer))) < 0) {
			printf("Error writing file data to socket\n");
			return;
		}
		bzero(buffer, BUFFER_SIZE);
	}
	return;
}

void recv_file(int sockfd) {

	char buffer[BUFFER_SIZE];
	FILE *fp;
	int n, num_bytes, count = 0;

	//Read filesize
	if((n = read(sockfd, buffer, BUFFER_SIZE - 1)) < 0) {
		printf("Error reading from socket\n");
		return;
	}
	num_bytes = atoi(buffer);

	//Read filename and open a file pointer
	if((n = read(sockfd, buffer, BUFFER_SIZE - 1)) < 0) {
		printf("Error reading from socket\n");
		return;
	}
	fp = fopen(buffer, "w");

	//Loop until num_bytes have been read and write them to the file
	while(count < num_bytes) {

		if((n = read(sockfd, buffer, BUFFER_SIZE - 1)) < 0) {
			printf("Error reading from socket\n");
			return;
		}
		fprintf(fp, "%s", buffer);
		bzero(buffer, BUFFER_SIZE);
		count += n;
	}
	fclose(fp);
	return;
}

int main() {

	//used to read in and parse user commands
	//char buffer[BUFFER_SIZE];
	char *buffer, *command;
	buffer = (char *) malloc(BUFFER_SIZE);
	
	//used for connecting socket
	struct sockaddr_in serv_addr;
    	struct hostent *server;
	char *ip, *tempno, *filename;
	int sockfd, portno, n;

	printf("Client starting...\n");

	while(!quitflag) {
		
		//Reset buffer to default size and clear buffer
		buffer = (char *) realloc(buffer, BUFFER_SIZE);
		bzero(buffer, BUFFER_SIZE);

		//Print prompt and read in user input checking for fgets error
		printf(">");
		if(fgets(buffer, BUFFER_SIZE, stdin) < 0) {
			perror("Error reading in command. Closing client\n");
			exit(1);
		}

		//Use strtok to get just the command issued and not args
		if((command = strtok(buffer, " ")) == NULL) {
			printf("Error tokenizing command please enter proper command\n");
		}

		//Convert command string to all lowercase to scrub user input
		for(int i = 0; command[i]; i++) {
			command[i] = tolower(command[i]);
		}

		//Determine what input was given by user and execute command
		
		printf("tokenized command: %s\n", command);
		//Connect command
		if(!(strcmp(command, "connect"))) {
			
			//Get ip and port number from user input checking for errors
			if((ip = strtok(NULL, " ")) == NULL) {
				printf("Error tokenizing server ip please enter -> CONNECT <ip/name> <server port>\n");
				continue;
			}
			if((tempno = strtok(NULL, " ")) == NULL) {
				printf("Error tokenizing port number please enter -> CONNECT <ip/name> <server port>\n");
				continue;
			}
			portno = atoi(tempno);

			//Open socket and gethostname checking for errors
			if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
				printf("Error opening socket\n");
				continue;
			}
			if((server = gethostbyname(ip)) == NULL) {
				printf("Error: No such host\n");
				continue;
			}

			//Attempt to connect socket
			bzero((char *) &serv_addr, sizeof(serv_addr));
    			serv_addr.sin_family = AF_INET;
    			bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   			serv_addr.sin_port = htons(portno);

			if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
       				printf("Error connecting socket\n");
				continue;
			}

			//Set connection flag because other commands should require connection first
			connectflag = 1;
			printf("Connected to %s:%d\n", ip, portno);
		}

		//List command
		else if(!(strcmp(command, "list\n"))) {
			
			if(!connectflag) {
				printf("Please connect to server first with connect command\n");
				continue;
			}

			//Write command to server
			n = write(sockfd, "list", 5);
			if (n < 0) {
				printf("Error sending list command\n");
				continue;
			}

			//Read response length from server
			char* response_len = (char *) malloc(32);
			n = read(sockfd, response_len, 5);
		       	if (n < 0) {
				printf("Error reading from socket\n");
				continue;
			}
		
			//Read file names and print ensuring that all bytes sent by server are read
			buffer = (char *) realloc(buffer, atoi(response_len));
			n = read(sockfd, buffer, atoi(response_len));
		       	if (n < 0) {
				printf("Error reading from socket\n");
				continue;
			}
			else if (n != atoi(response_len)) {
				printf("Did not receive entire message from server -- %d of %d bytes\n", n, atoi(response_len));
			}	
			printf("--LIST--\n%s\n", buffer);	
			free(response_len);
		}

		//Retrieve command
		else if(!(strcmp(command, "retrieve"))) {
			
			if(!connectflag) {
				printf("Please connect to server first with connect command\n");
				continue;
			}

			//Write command to server
			n = write(sockfd, "retrieve", 9);
			if (n < 0) {
				printf("Error sending retrieve command\n");
				continue;
			}

			//Get filename from user input checking for errors
			if((filename = strtok(NULL, "\n")) == NULL) {
				printf("Error tokenizing filename please enter -> RETRIEVE <filename>\n");
				continue;
			}

			//Write desired filename to server
			n = write(sockfd, filename, strlen(filename));
			if (n < 0) {
				printf("Error sending file name\n");
				continue;
			}

			//Clear buffer to get rid of command
		        bzero(buffer, strlen(buffer));

		        recv_file(sockfd);
		}

		//Store command
		else if(!(strcmp(command, "store"))) {
		
			if(!connectflag) {
				printf("Please connect to server first with connect command\n");
				continue;
			}

			//Write command to server
			n = write(sockfd, "store", 6);
			if (n < 0) {
				printf("Error sending store command\n");
				continue;
			}

			//Get filename from user input checking for errors
			if((filename = strtok(NULL, "\n")) == NULL) {
				printf("Error tokenizing filename please enter -> STORE <filename>\n");
				continue;
			}

			send_file(filename, sockfd);

		}

		//Quit command
		else if(!(strcmp(command, "quit\n"))) {
			
			quitflag = 1;
			printf("Closing socket and quitting...\n");

			//Write command to server
			n = write(sockfd, "quit", 5);
			if (n < 0) {
				printf("Error sending quit command\n");
				continue;
			}

			//Close socket connection
			if(connectflag) {
				shutdown(sockfd, SHUT_RDWR);
			}

		}
		//User entered command incorrectly or non-existent command
		else {
			if(!(strcmp(command, "connect\n"))) {
				printf("Connect command requires arguments (e.g., CONNECT <ipaddr> <port number>)\n");
			}
			else if(!(strcmp(command, "retrieve\n"))) {
				printf("Retrieve command requires arguments (e.g., RETRIEVE <filename>)\n");
			}
			else if(!(strcmp(command, "store\n"))) {
				printf("Store command requires arguments (e.g., STORE <filename>)\n");
			}
			else {
				printf("Command not recognized please be sure to enter in command correctly\n");
			}
		}
	}

	free(buffer);

	return 0;
}
