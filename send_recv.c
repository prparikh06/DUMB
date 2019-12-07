#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>


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

int readMessage(int connfd){

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
	printf("received message from client: %s\n", finalMessage);

}



