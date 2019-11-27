#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 


//server socket port number: random number between 4096 and 65k

int main(int argc, char** argv){

	int sockfd, connectionfd;
	struct sockaddr_in server_addy, client;	



	int port_num = atoi(argv[1]);
	
	//TODO listen and accept here
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd == -1){
		printf("error");
		return 0;
	}

	bzero(&server_addy, sizeof(server_addy));
	
	server_addy.sin_family = AF_INET;
	server_addy.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addy.sin_port = htons(port_num);

	if (bind(sockfd, (struct sockaddr*)&server_addy,sizeof( server_addy)) != 0){

		printf("error in binding\n");
		return 0;
	}
	if (listen (sockfd, 3) != 0){

		printf("error in listening\n");
		return 0;
	}
	
	int size = sizeof(struct sockaddr_in);
	
	connectionfd = accept(sockfd, (struct sockaddr*) &client,(socklen_t*) &size );
	if (connectionfd < 0) {
		printf("error in accepting\n");
		return 0;
	}

	printf("ayyy the server accepted!!\n");

	

	return 0;	


}
