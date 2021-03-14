#include <stdio.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <libgen.h>

#define BUFFER_SIZE 4096

int quitflag = 0;

void error(char *msg) {

    perror(msg);
    exit(1);
}

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


int main(int argc, char *argv[]) {

     int sockfd, newsockfd, portno, clilen;
     char *buffer;//, *filename;
     buffer = (char *) malloc(BUFFER_SIZE);
     struct sockaddr_in serv_addr, cli_addr;
     int n;

     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

     //Create socket
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) {
        error("ERROR opening socket");
     }

     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);

     //Bind socket
     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) { 
	     error("ERROR on binding");
     }
    
     //Set socket to passive mode to wait for client connection
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
     if (newsockfd < 0) {
	     error("ERROR on accept");
     }

     printf("Connected to client\n");

     while(!quitflag) {

	     //Reset buffer to default size and clear buffer
	     buffer = (char *) realloc(buffer, BUFFER_SIZE);
	     bzero(buffer, BUFFER_SIZE);

	     //Read command sent from client checking for error
	     n = read(newsockfd, buffer, BUFFER_SIZE - 1);
	     if (n < 0) {
		    error("ERROR reading from socket");
	     }

	     printf("Received message from client: %s\n",buffer);

	     //List command received
	     if(!(strcmp(buffer, "list"))) {

		     //Clear buffer to get rid of command
		     bzero(buffer, strlen(buffer));

		     //Create temporary string for appending filenames to buffer one at a time
		     char *tmp;
		     tmp = (char *) malloc(BUFFER_SIZE);
		     DIR *d;				
  		     struct dirent *dir;

		     //Open current directory and read through filenames
		     d = opendir(".");
  		     if (d) {

			     //Copy file name to tmp, realloc buffer size to fit, strcat to end of buffer
   			     while ((dir = readdir(d)) != NULL) {
				     strcpy(tmp, dir->d_name);
				     buffer = (char *) realloc(buffer, strlen(buffer) + strlen(tmp) + 1);
				     strcat(buffer, tmp);
				     strcat(buffer, "\n");
			     }
			     closedir(d);
		     }

		     //Send length of string to error check on client side
		     char *response_len = (char *) malloc(32); 
		     sprintf(response_len, "%lu", strlen(buffer));
		     n = write(newsockfd, response_len, strlen(response_len));
		     if(n < 0) {
			     error("Error writing to socket");
		     }

		     //Send list of filenames
		     n = write(newsockfd, buffer, strlen(buffer));
		     if(n < 0) {
			     error("Error writing to socket");
		     }

		     free(response_len);
	     }

	     else if(!(strcmp(buffer, "retrieve"))) {

		     //Clear buffer to get rid of command
		     bzero(buffer, strlen(buffer));

		     //Read file name sent from client checking for error
	    	     n = read(newsockfd, buffer, BUFFER_SIZE - 1);
	     	     if (n < 0) {
		    	     error("ERROR reading from socket");
	     	     }

		     send_file(buffer, newsockfd);
	     }

	     else if(!(strcmp(buffer, "store"))) {

		     //Clear buffer to get rid of command
		     bzero(buffer, strlen(buffer));

		     recv_file(newsockfd);

		     printf("got store command\n");
	     }

	     else if(!(strcmp(buffer, "quit"))) {

		     quitflag = 1;
		     printf("Closing socket and quitting...\n");

		     //Close socket connection
		     shutdown(sockfd, SHUT_RDWR);
	     }


     }

     free(buffer);

     return 0; 
}
