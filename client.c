#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define BUFFER_SIZE 4096

int quitflag = 0;
int connectflag = 0;

int main() {

	//used to read in and parse user commands
	char buffer[BUFFER_SIZE];
	char *command;
	
	//used for connecting socket
	struct sockaddr_in serv_addr;
    	struct hostent *server;
	char *ip, *tempno;
	int sockfd, portno;

	printf("Client starting...\n");

	while(!quitflag) {
		
		//Clear buffer
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
			if((ip = strtok(buffer, " ")) == NULL) {
				printf("Error tokenizing server ip please enter -> CONNECT <ip/name> <server port>\n");
				continue;
			}
			if((tempno = strtok(buffer, " ")) == NULL) {
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


		}

		//List command
		else if(!(strcmp(command, "list\n"))) {
			printf("testing list\n");
		}

		//Retrieve command
		else if(!(strcmp(command, "retrieve"))) {
			printf("testing retrieve\n");
		}

		//Store command
		else if(!(strcmp(command, "store"))) {
			printf("testing store\n");
		}

		//Quit command
		else if(!(strcmp(command, "quit\n"))) {
			
			quitflag = 1;
			printf("Closing socket and quitting...\n");

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


	return 0;
}
