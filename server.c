#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "queue.h"

int readCommands(int connfd){

	char* message = malloc(1);

	for (;;){
		read(connfd,message,sizeof(message));
		printf("received message from client: %s\n", message);
		if (strcmp(message, "exit") == 0) break;
		bzero(message, sizeof(message));
		printf("send a message to the client: \n");
		int c, i = 0;
		int max = 1;
		while((c = getchar()) != '\n' && c != EOF){
			message[i++] = (char) c;
			if (i == max){
				max += 1;
				message = realloc(message,max);
			}
		}
		message[i] = '\0';			


		write(connfd,message,sizeof(message));
		if (strcmp(message, "exit") == 0) break;
	


	}



}
int main(int argc, char* argv[]) {
/*
	struct Node* queue = (struct Node*) malloc(sizeof(struct Node));
	enqueue(&queue, "parikh");
	enqueue(&queue,"priya");
	printList(queue);
    printf("size of queue should be 2: %d\n", size);
*/

    int sockfd, connfd;
    struct sockaddr_in servaddr, clientaddr;

    int port_num = atoi(argv[1]);

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
       return 0;
    }

    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port_num);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
	printf("binding failed\n");
	return 0;
    }

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0) {
	printf("Listening failed...\n");
	return 0;
    }
    else
	printf("Listening..\n");

    int size = sizeof(clientaddr);

    // Accept the data packet from client and verification
    connfd = accept(sockfd, (struct sockaddr*)&clientaddr, &size);
    if (connfd < 0) {
	printf("could not accept client :(\n");
	return 0;
    }
	readCommands(connfd);
	close(sockfd);

}
