#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#define BUFFER_SIZE 4096

int quitflag = 0;

void error(char *msg) {

    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {

     int sockfd, newsockfd, portno, clilen;
     char *buffer;
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

		     //Clear buffer
		     bzero(buffer, strlen(buffer));

		     char *tmp;
		     tmp = (char *) malloc(BUFFER_SIZE);
		     DIR *d;
  		     struct dirent *dir;

		     //Open directory and read through filenames
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

	     }

	     else if(!(strcmp(buffer, "store"))) {

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
