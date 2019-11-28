#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 

//server socket port number: random number between 4096 and 65k

void func(int sockfd) 
{ 
    char buff[100]; 
    int n; 
    // infinite loop for chat 
    for (;;) { 
        bzero(buff, 100); 
  
        // read the message from client and copy it in buffer 
        read(sockfd, buff, sizeof(buff)); 
        // print buffer which contains the client contents 
        printf("From client: %s\t To client : ", buff); 
        bzero(buff, 100); 
        n = 0; 
        // copy server message in the buffer 
        while ((buff[n++] = getchar()) != '\n') 
            ; 
  
        // and send that buffer to client 
        write(sockfd, buff, sizeof(buff)); 
  
        // if msg contains "Exit" then server exit and chat ended. 
        if (strncmp("exit", buff, 4) == 0) { 
            printf("Server Exit...\n"); 
            break; 
        } 
    } 
} 

int main(int argc, char* argv[]) { 
    int sockfd, connfd; 
    struct sockaddr_in servaddr, clientaddr; 

    int port_num = atoi(argv[1]);
  
    // socket create and verification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
       return 0; 
    } 
    else
        printf("Socket successfully created..\n"); 
    
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
    else
        printf("accepted!!!\n"); 
  
    // Function for chatting between client and server 
    func(connfd); 
  
    // After chatting close the socket 
    close(sockfd); 
} 
