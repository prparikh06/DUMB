#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h>

#include <sys/types.h> 
#include <netinet/in.h> 


//52613
int numAttempts = 0;
char* commandOptions[] = {"quit", "create", "delete", "open", "close", "next", "put"};


int checkCommand(char* str){

	for (int i = 0; i < 7; i++){
		if(strcmp(str,commandOptions[i]) == 0)
			return 1;
	}
	return 0;

}


int main(int argc, char* argv[] ){
	
	char* hostname = argv[1];
	int port_num = atoi(argv[2]);
	
	//TODO connect to server here
	
	int sockfd = socket(AF_INET, SOCK_STREAM,0);
	if (sockfd < 0){ //TODO Error
		printf("error\n");
		return 0;
	}
	
	char* ip;
	struct hostent *host = gethostbyname(hostname);
	
	if (host == NULL){ //TODO error
		printf("hostname error\n");
		return 0;

	}
	
	struct sockaddr_in server_addy;
	bzero(&server_addy, sizeof(server_addy));
	
	

	server_addy.sin_addr = *(struct in_addr *) host->h_addr_list;


	server_addy.sin_family = AF_INET;
	//server_addy.sin_addr.s_addr = inet_addr(ip);
	//server_addy.sin_addr = *((struct in_addr *) host->h_addr_list[0]);
	//memcpy(&(server_addy.sin_addr.s_addr), host->h_addr_list[0], host->h_length);
	server_addy.sin_port = htons(port_num);
	
	if(connect(sockfd, (struct sockaddr *) &server_addy, sizeof(server_addy)) < 0){
		printf("could not be connected :(\n");
		return 0;
	}
	
	
	
	//printf("connected!!!!\n");

	

	//TODO do stuff

	
	/*	
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
	*/
	return 0;

}
