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

int main() {

	char *buffer, *command;
	printf("Client starting...\n");

	while(!quitflag) {
		
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

		//Determine what input was given by user
		if(!(strcmp(buffer, "connect\n"))) {

		}
		else if(!(strcmp(buffer, "list\n"))) {

		}
		else if(!(strcmp(buffer, "retrieve\n"))) {

		}
		else if(!(strcmp(buffer, "store\n"))) {

		}
		else if(!(strcmp(buffer, "quit\n"))) {
			
			//close connection
			quitflag = 1;
		}
		else {
			printf("Command not recognized please be sure to enter in command correctly\n");
		}
	}


	return 0;
}
