#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "queue.h"

int readCommands(int connfd){

	//char* message = malloc(100);
	//message[0] = '\0';
	
	//char message[1024];
	int recv_size = 0;
	recv(connfd, &recv_size, sizeof(recv_size),0);
	printf("size of the incoming message: %d\n", ntohl(recv_size));
	int len = ntohl(recv_size);
	//char* message = (char*) malloc(ntohl(recv_size));
	//printf("size of message is: %d\n", sizeof(message));
	char message[len];
	printf("sze of message should be %d, is actually %d\n", len, sizeof(message));
	int read= 0; 
	char finalMessage[len+1];
	//finalMessage[0] = 0;
	
	while(read < len){
		read += recv(connfd, message, len,0);
		printf("read = %d, goal = %d\n", read, len);
		strcat(finalMessage,message);
		

	}
	
	

	finalMessage[len] = '\0';

	//printf("status of recv: %d\n", status);
	printf("received message from client: %s\n", finalMessage);

	//bzero(message, sizeof(message));
	//printf("send a message to the client: \n");
	/*int c, i = 0;
	int max = 1;
	while((c = getchar()) != '\n' && c != EOF){
		message[i++] = (char) c;
		if (i == max){
			max = i+1;
			message = realloc(message,max);
		}
	}
	message[i] = '\0';			
	*/
	/*
	scanf("%m[^\n]", &message);
	printf("sending message: %s\n", message);	
	
	write(connfd,message,sizeof(message));
	*/
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
