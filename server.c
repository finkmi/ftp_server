#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
	     bzero(buffer,1024);
	     n = read(newsockfd, buffer, BUFFER_SIZE - 1);
	     
	     if (n < 0) {
		    error("ERROR reading from socket");
	     }

	     printf("Received message from client: %s\n",buffer);
	     printf("Enter response: ");
	     bzero(buffer, 1024);
	     fgets(buffer,1024,stdin);
	     n = write(newsockfd,buffer,strlen(buffer));

	     if (n < 0) {
		     error("ERROR writing to socket");
	     }
     }

     free(buffer);

     return 0; 
}
