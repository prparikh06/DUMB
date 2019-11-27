#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 

//server socket port number: random number between 4096 and 65k
/*
void func(int sockfd){
	char buff[100];
	int n;
	for (;;){
		bzero(buff,100);
		read(sockfd,buff,sizeof(buff));
		printf("From clien: %s\tTo client:", buff);
		bzero(buff,100);
		n = 0;
		while((buff[n++] = getchar())!='\n');
		write(sockfd,buff,sizeof(buff));
		if (strncmp("exit",buff,4) == 0){
			printf("server exit\n");
			break;
		}
	}

}
*/

void checkCommands(int connfd){
	char command[100];
	for (;;){
		read(connfd, command, sizeof(command));
        
        if(strcmp(command,"exit") == 0) break;	

		printf("client sent command: %s\n", command);
       	
	}
}	

int main(int argc, char* argv[]){

	int port_num = atoi(argv[1]);
	
	int sockfd, connfd; 
    struct sockaddr_in serv_addr, client_addr; 
  
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 

    if (sockfd < 0) { 
        printf("could not create socket\n"); 
        return 0; 
    } 
    
    bzero((char*)&serv_addr, sizeof(serv_addr)); 
    
    //initialize server socket
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_addr.s_addr = INADDR_ANY; 
    serv_addr.sin_port = htons(port_num); 
  
    //bind 
    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) { 
        printf("socket binding error...\n"); 
        return 0;
    } 

    //listen
    if (listen(sockfd, 5) != 0){
        printf("listening error\n");
        return 0;
    } 
    
    int size = sizeof(client_addr); 
  
    //accept the connection
    if (connfd = accept(sockfd, (struct sockaddr*)&client_addr, &size) != 0) { 
        printf("connecting error\n"); 
        return 0;
    } 
    
	checkCommands(connfd);

}
