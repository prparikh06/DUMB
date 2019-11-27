#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 

//server socket port number: random number between 4096 and 65k


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



int main(int argc, char** argv){

	unsigned short PORT; 
	sscanf(argv[1],"hi",&PORT);

	int sockfd, connfd, len; 
    struct sockaddr_in server_addy, cli; 
  
    // socket create and verification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        return 0; 
    } 
    else
        printf("Socket successfully created..\n"); 
    bzero(&server_addy, sizeof(server_addy)); 
  
    // assign IP, PORT 
    server_addy.sin_family = AF_INET; 
    server_addy.sin_addr.s_addr = htonl(INADDR_ANY); 
    server_addy.sin_port = htons(PORT); 
  
    // Binding newly created socket to given IP and verification 
    if ((bind(sockfd, (struct sockaddr*)&server_addy, sizeof(server_addy))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully binded..\n"); 
  
    // Now server is ready to listen and verification 
    if ((listen(sockfd, 5)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } 
    else
        printf("Server listening..\n"); 
    len = sizeof(cli); 
  
    // Accept the data packet from client and verification 
    connfd = accept(sockfd, (struct sockaddr*)&cli, &len); 
    if (connfd < 0) { 
        printf("server acccept failed...\n"); 
        exit(0); 
    } 
    else
        printf("server acccept the client...\n"); 
  


	func(connfd);

		


}
