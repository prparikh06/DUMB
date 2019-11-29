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


int checkCommand(char* str){

	for (int i = 0; i < 7; i++){
		if(strcmp(str,commandOptions[i]) == 0)
			return 1;
	}
	return 0;

}

void func(int sockfd)
{
    char buff[100];
    int n;
    for (;;) {
        bzero(buff, sizeof(buff));
        printf("Enter the string : ");
        n = 0;
        while ((buff[n++] = getchar()) != '\n')
            ;
        write(sockfd, buff, sizeof(buff));
        bzero(buff, sizeof(buff));
        read(sockfd, buff, sizeof(buff));
        printf("From Server : %s", buff);
        if ((strncmp(buff, "exit", 4)) == 0) {
            printf("Client Exit...\n");
            break;
        }
    }
}


void readCommands(int sockfd){

	char message[1024];
	for (;;){
		bzero(message,sizeof(message));
		printf("Enter a message...\n");
		scanf("%s", message);
		printf("message is: %s\n", message);
		write(sockfd, message,sizeof(message));
		if (strcmp(message,"exit") == 0) break;
		bzero(message,sizeof(message));
		read(sockfd,message,sizeof(message));
		printf("client sent message: %s\n", message);
		if (strcmp(message, "exit") == 0) break;

	}
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

    // connect the client socket to server socket
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connecting failed :(\n");
        exit(0);
    }
    else
        printf("connected!!!\n");

    readCommands(sockfd);

    close(sockfd);
}

