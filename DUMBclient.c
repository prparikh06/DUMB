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
char* clientCommands[] = {"HELLO", "GDBYE", "CREAT", "OPNBX", "NXTMG", "PUTMG", "DELBX", "CLSBX"};
int connected = 0;


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

void handleOpen(int sockfd){
    char message[1024];
    bzero(message,sizeof(message));
    strcpy(message,clientCommands[3]);
    write(sockfd, message,sizeof(message)); //SEND OPENBX
    bzero(message,sizeof(message));
    printf("Okay, open which message box?\n");
    printf("open:> ");
    bzero(message,sizeof(message));
    scanf("%s", message);
    write(sockfd, message,sizeof(message)); //SEND box name
    printf("opening box: %s\n", message);
    bzero(message,sizeof(message));
    read(sockfd,message,sizeof(message)); //WAIT FOR SUCCESS
    if (strcmp(message, "OK!") == 0){
        printf("Success! Message box 'mybox' is now open.\n");
        printf("> ");
    }else{
        printf("Error. Command was unsuccessful, please try again.\n");
    }
}

void handleCreate(int sockfd){
    char message[1024];
    bzero(message,sizeof(message));
    strcpy(message,clientCommands[2]);
    write(sockfd, message,sizeof(message)); //SEND CREATEBX
    bzero(message,sizeof(message));
    printf("Okay, create a box name!\n");
    printf("create:> ");
    bzero(message,sizeof(message));
    scanf("%s", message);
    char boxName[1024]; strcpy(boxName,message);
    write(sockfd, message,sizeof(message)); //SEND box name
    printf("creating box: %s\n", message);
    bzero(message,sizeof(message));
    read(sockfd,message,sizeof(message)); //WAIT FOR SUCCESS
    if (strcmp(message, "OK!") == 0){
        printf("Success! Message box %s has been created.\n", boxName);
    }else if(strcmp(message, "ER:EXIST") == 0){
        printf("Failed! Message box %s already exists.\n", boxName);
    }
    else{
        printf("Error. Command was unsuccessful, please try again.\n");
    }
}


void readCommands(int sockfd){

	char message[1024];
	for (;;){
		bzero(message,sizeof(message));
		if(connected == 0){
            strcpy(message,clientCommands[0]);
            write(sockfd, message,sizeof(message)); //UPON CONNECTION: CLIENT SENDS HELLO
            bzero(message,sizeof(message));
            read(sockfd,message,sizeof(message)); //AFTER RECEIVING READY FROM SERVER, CONTINUE TO RECEIVE MORE COMMANDS
            if(strcmp(message, "HELLO DUMBv0 ready!") == 0){
                printf("HELLO DUMBv0 ready!\n");
                connected = 1;
            }else{
                printf("Connection failed\n");
                break;
            }
		}
		printf("Enter a message...\n");
		scanf("%s", message);
		printf("message is: %s\n", message);
		if (strcmp(message, "exit") == 0) break;

		if (strcmp(message,"create") == 0){
            handleCreate(sockfd);
            continue;
		}
		if (strcmp(message,"delete") == 0){
            printf("handle delete\n");
            continue;
		}
		//IF COMMAND IS OPEN, FOLLOW UP
		if (strcmp(message,"open") == 0){
            handleOpen(sockfd);
            continue;
		}
		if (strcmp(message,"close") == 0){
            printf("handle close\n");
            continue;
		}
		if (strcmp(message,"next") == 0){
            printf("handle next\n");
            continue;
		}
		if (strcmp(message,"put") == 0){
            printf("handle put\n");
            continue;
		}
		if (strcmp(message,"quit") == 0){
            printf("handle quit\n");
            continue;
		}
		bzero(message,sizeof(message));
		scanf("%s", message);
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

    char command[100];
    printf("Enter a message...\n");
    scanf("%s", command);
    if (strcmp(command,"hello") != 0){
        printf("Goodbye\n"); //will deal with error cases laterrrr
        exit(0);
    }

    // connect the client socket to server socket
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connecting failed :(\n");
        exit(0);
    }

    readCommands(sockfd);

    close(sockfd);
}

