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

int sendMessage(int sockfd, char* message){
	int len = strlen(message);	
	int converted = htonl(len);
	
	//send size and string to server
	send(sockfd, &converted, sizeof(converted),0);
	int size = strlen(message);
	
	int sent = 0;
	while (sent < size){
		sent+=send(sockfd, message, size,0);
		printf("client sent: %d of %d\n", sent, size);
		if (sent < 0) return -1;
	}
	
	return 0;
}
int readCommands(int sockfd){
	//char* message = malloc(1);
	//message[0] = '\0';
	printf("please send a message!\n");

	char* message;
	scanf("%m[^\n]",&message);
	printf("message = %s\tlength = %d\n", message,strlen(message));
	return sendMessage(sockfd, message);
	/*int len = strlen(message);	
	int converted = htonl(len);
	
	//send size and string to server
	send(sockfd, &converted, sizeof(converted),0);
	int size = strlen(message);
	
	int sent = 0;
	while (sent < size){
		sent+=send(sockfd, message, size,0);
		printf("client sent: %d of %d\n", sent, size);
		if (sent < 0) return -1;
	}
	*/
	return 0;
	
}

int main(int argc, char* argv[]) {
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

	char* hostname = argv[1];
	int port_num = atoi(argv[2]);

    // socket create and varification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        return 0;
    }

    bzero(&servaddr, sizeof(servaddr));

  	struct hostent *host = gethostbyname(hostname);

	if (host == NULL){ //TODO error
		printf("hostname error\n");
		return 0;
	}
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
	servaddr.sin_addr = *(struct in_addr* )* host->h_addr_list;
    servaddr.sin_port = htons(port_num);
/*
    char command[100];
    printf("Enter a message...\n");
    scanf("%s", command);
    if (strcmp(command,"hello") != 0){
        printf("Goodbye\n"); //will deal with error cases laterrrr
        exit(0);
    }
*/
    // connect the client socket to server socket
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connecting failed :(\n");
        exit(0);
    }

    readCommands(sockfd);

    close(sockfd);
}

