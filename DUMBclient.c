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


void handleOpen(int sockfd){
    char message[1024];
    bzero(message,sizeof(message));
    //strcpy(message,clientCommands[3]);
    //write(sockfd, message,sizeof(message)); //SEND OPENBX
    //bzero(message,sizeof(message));
    printf("Okay, open which message box?\n");
    printf("open:> ");
    //bzero(message,sizeof(message));
    scanf("%s", message);
    char boxName[1024]; strcpy(boxName,message);
    bzero(message,sizeof(message));
    sprintf(message, "OPNBX %s", boxName);
    printf("message sending: %s\n", message);

    write(sockfd, message,sizeof(message)); //SEND box name
    printf("opening box: %s\n", boxName);
    bzero(message,sizeof(message));
    read(sockfd,message,sizeof(message)); //WAIT FOR SUCCESS
    if (strcmp(message, "OK!") == 0){
        printf("Success! Message box '%s' is now open.\n", boxName);
        //printf("> ");
    }else{
        printf("Error. Command was unsuccessful, please try again.\n");
    }
}

void handleCreate(int sockfd){
    char message[1024];
    bzero(message,sizeof(message));
    //strcpy(message,clientCommands[2]);
    //write(sockfd, message,sizeof(message)); //SEND CREATEBX
    //bzero(message,sizeof(message));
    printf("Okay, create a box name!\n");
    printf("create:> ");
    //bzero(message,sizeof(message));
    scanf("%s", message);
    char boxName[1024]; strcpy(boxName,message);
    //int boxLen = strlen(boxName)+1;
    bzero(message,sizeof(message));
    sprintf(message, "CREAT %s", boxName);
    printf("message sending: %s\n", message);

    write(sockfd, message,sizeof(message)); //SEND box name
    printf("creating box: %s\n", boxName);
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

void handleClose(int sockfd){
    char message[1024];
    bzero(message,sizeof(message));
    //strcpy(message,clientCommands[7]);
    //write(sockfd, message,sizeof(message)); //SEND CLSBX
    //bzero(message,sizeof(message));
    printf("Okay, close which box?\n");
    printf("close:> ");
    //bzero(message,sizeof(message));
    scanf("%s", message);
    char boxName[1024]; strcpy(boxName,message);
    bzero(message,sizeof(message));
    sprintf(message, "CLSBX %s", boxName);
    printf("message sending: %s\n", message);

    write(sockfd, message,sizeof(message)); //SEND box name
    printf("closing box: %s\n", boxName);
    bzero(message,sizeof(message));
    read(sockfd,message,sizeof(message)); //WAIT FOR SUCCESS
    if (strcmp(message, "OK!") == 0){
        printf("Success! Message box %s has been closed.\n", boxName);
    }else if(strcmp(message, "ER:NOOPN") == 0){
        printf("Failed! You do not currently have the box opened, so you can't close it.\n", boxName);
    }
    else{
        printf("Error. Command was unsuccessful, please try again.\n");
    }
}

int sendMessage(int sockfd, char* message){
	int len = strlen(message);
	int converted = htonl(len);

	//send size and string to server
	//send(sockfd, &converted, sizeof(converted),0);
	int size = strlen(message);

	int sent = 0;
	while (sent < size){
		//sent+=send(sockfd, message, size,0);
		sent+= write(sockfd,message,size);
		printf("client sent: %d of %d\n", sent, size);
		if (sent < 0) return -1;
	}

	return 0;
}

void handlePut(int sockfd){
    char message[1024];
    bzero(message,sizeof(message));
    //strcpy(message,clientCommands[5]);
    //write(sockfd, message,sizeof(message)); //SEND PUTMG
    //bzero(message,sizeof(message));
    char msg[1024];
    //char* msg;
    //bzero(msg,sizeof(msg));
    printf("Okay, enter your message:\n");
    printf("put:> ");
    scanf("%s", &msg);
    //scanf("%m[^\n]",&msg);
    int numBytes = strlen(msg)+1;
    printf("num of bytes: %d\n", numBytes);
    //strcpy(message, "PUTMG!"); strcat(message, itoa(numBytes)); strcat(message, "!"); strcat(message, msg);
    char* theMesseage = malloc(11+numBytes);
    sprintf(theMesseage, "PUTMG!%d!%s", numBytes, msg);
    //sprintf(message, "PUTMG!%d!%s", numBytes, msg);
    //*message = clientCommands[5]; message[5] = '!'; message[6] = (char)numBytes; message[7] = '!'; message[8] = msg;
    printf("message: %s\n", theMesseage);
    //write(sockfd, message,sizeof(message)); //SEND MSG
    sendMessage(sockfd,theMesseage);
    char weWant[1024]; sprintf(weWant,"OK!%d", numBytes);
    printf("expecting: %s\n", weWant);
    bzero(message,sizeof(message));
    read(sockfd,message,sizeof(message)); //WAIT FOR SUCCESS
    if (strcmp(message, weWant) == 0){
        printf("Success! Message has been put.\n");
    }else if(strcmp(message, "ER:NOOPN") == 0){
        printf("Failed! You do not currently have the box opened, so you can't close it.\n");
    }
    else{
        printf("Error. Command was unsuccessful, please try again.\n");
    }

}

void handleNext(int sockfd){
    char message[1024];
    bzero(message,sizeof(message));
    strcpy(message,clientCommands[4]);
    write(sockfd, message,sizeof(message)); //SEND NXTMG
    bzero(message,sizeof(message));
    read(sockfd,message,sizeof(message)); //WAIT FOR MESSAGE
    if (strncmp(message, "OK!", 3) == 0){
        printf("Success! Message received: %s\n", message);
    }else if(strcmp(message, "ER:NOOPN") == 0){
        printf("Failed! You do not currently have the box opened, so you can't close it.\n");
    }
    else{
        printf("Error. Command was unsuccessful, please try again.\n");
    }

}

void handleDelete(int sockfd){
    char message[1024];
    bzero(message,sizeof(message));
    printf("Okay, delete which box?\n");
    printf("delete:> ");
    //bzero(message,sizeof(message));
    scanf("%s", message);
    char boxName[1024]; strcpy(boxName,message);
    bzero(message,sizeof(message));
    sprintf(message, "DELBX %s", boxName);
    printf("message sending: %s\n", message);

    write(sockfd, message,sizeof(message)); //SEND box name
    printf("deleting box: %s\n", boxName);
    bzero(message,sizeof(message));
    read(sockfd,message,sizeof(message)); //WAIT FOR SUCCESS
    if (strcmp(message, "OK!") == 0){
        printf("Success! Message box %s has been deleted.\n", boxName);
    }else if(strcmp(message, "ER:NEXST") == 0){
        printf("Failed! Box %s does not exist.\n", boxName);
    }else if(strcmp(message, "ER:OPEND") == 0){
        printf("Failed! Box %s is currently open.\n", boxName);
    }else if(strcmp(message, "ER:NOTMT") == 0){
        printf("Failed! Box %s is not empty.\n", boxName);
    }
    else{
        printf("Error. Command was unsuccessful, please try again.\n");
    }
}

void handleQuit(int sockfd){
    char message[1024];
    bzero(message,sizeof(message));
    strcpy(message,clientCommands[1]);
    write(sockfd, message,sizeof(message)); //SEND GDBYE
    //bzero(message,sizeof(message));
    //read(sockfd,message,sizeof(message)); //WAIT FOR MESSAGE
}


void readCommands(int sockfd){

	char message[1024];
	for (;;){
		//bzero(message,sizeof(message));
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
		printf("> ");
		bzero(message,sizeof(message));
		scanf("%s", message);
		printf("message is: %s\n", message);
		if (strcmp(message, "quit") == 0){
            handleQuit(sockfd);
            break;
		}
		if (strcmp(message,"create") == 0){
            handleCreate(sockfd);
            continue;
		}
		if (strcmp(message,"delete") == 0){
            handleDelete(sockfd);
            continue;
		}
		//IF COMMAND IS OPEN, FOLLOW UP
		if (strcmp(message,"open") == 0){
            handleOpen(sockfd);
            continue;
		}
		if (strcmp(message,"close") == 0){
            handleClose(sockfd);
            continue;
		}
		if (strcmp(message,"next") == 0){
            handleNext(sockfd);
            continue;
		}
		if (strcmp(message,"put") == 0){
            handlePut(sockfd);
            continue;
		}
		if (strcmp(message,"help") == 0){
            printf("Available commands:\nquit\ncreate\ndelete\nopen\nclose\nnext\nput\n");
            continue;
		}
		printf("This is not a valid command. Try again!\n"); continue;
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
	while(numAttempts < 3){
		if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
	        printf("connecting failed :(\n");
	        exit(0);
		numAttempts ++;
		continue;
    		}
		else break;
	}

    readCommands(sockfd);

    close(sockfd);
}

