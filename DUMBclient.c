#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>

int numAttempts = 0;
char* commandOptions[] = {"quit", "create", "delete", "open", "close", "next", "put"};


int checkCommand(char* str){

	for (int i = 0; i < 7; i++){
		if(strcmp(str,commandOptions[i]) == 0)
			return 1;
	}
	return 0;

}


int main(int argc, char** argv ){
	
	char* hostname = argv[1];
	int port_num = atoi(argv[2]);
	
	//TODO connect to server here
		
	//accpet some commands (HELLO)
	printf("HELLO!\n");
	int acceptCommands = 1;
	while(acceptCommands != 0){ //while commands are getting inputted
		char command[100];
		scanf("%s", command);
		if (checkCommand(command) != 1)
			acceptCommands = 0;
		

		printf("command is %s\n", command);
		


	}
	printf("exitted loop\n");	

	return 0;

}
