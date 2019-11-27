#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h>
#include <sys/types.h> 
#include <netinet/in.h> 
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <signal.h>

//52613
int numAttempts = 0;
char* commandOptions[] = {"quit", "create", "delete", "open", "close", "next", "put"};

/*
void func(int sockfd){
	char buff[100];
	int n;
	for (;;){
		bzero(buff,sizeof(buff));
		printf("enter a string : ");
		n = 0;
		while((buff[n++] = getchar()) != '\n');
		write(sockfd,buff,sizeof(buff));
		bzero(buff,sizeof(buff));
		read(sockfd,buff,sizeof(buff));
		printf("From Server:%s ", buff);
		if ((strncmp(buff,"exit",4)) == 0) {
			printf("exiting...\n");
			break;
		}
	}
}
*/

int isAcceptedCommand(char* str){

	for (int i = 0; i < 7; i++){
		if(strcmp(str,commandOptions[i]) == 0)
			return 1;
	}
	return 0;
}

void checkCommands(int sockfd){
	int acceptCommands = 1;
	while(acceptCommands != 0){ //while commands are getting inputted
		char command[100];
		scanf("%s", command);
		/*if (isAcceptedCommand(command) != 1)
			acceptCommands = 0;
		*/

		printf("command is %s\n", command);

		write(sockfd, command, sizeof(command));
		if (strcmp(command,"exit") == 0) break;
	
	}
	printf("exitted loop\n");	
}


int main(int argc, char* argv[] ){
	
	char* hostname = argv[1];
	int port_num = atoi(argv[2]);
	struct sockaddr_in serv_addr;

	int sockfd = socket(AF_INET, SOCK_STREAM,0);
	
	if (sockfd < 0){ 
		printf("could not create socket\n"); 
        return 0; 
	}
	
	struct hostent *host = gethostbyname(hostname);
	
	if (host == NULL){ 
		printf("hostname error\n");
		return 0;

	}
	
	bzero(&serv_addr, sizeof(serv_addr));
	
	//initialize socket
	serv_addr.sin_addr = *(struct in_addr* )* host->h_addr_list;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port_num);
	
	while(1){
		 if (numAttempts == 3){
			printf("3 attempts\n");
			return 0;
		}
		
		if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0){
			printf("could not be connected :(\n");
			numAttempts++;
		}
		else break; //connect successful

	}
	
	//accpet some commands (HELLO)
	printf("HELLO!\n");

	checkCommands(sockfd);

	close(sockfd);
}
