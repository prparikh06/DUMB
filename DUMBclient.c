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
#include <arpa/inet.h>
#include <ctype.h>


//52613
int numAttempts = 0;
char* commandOptions[] = {"quit", "create", "delete", "open", "close", "next", "put"};
char* clientCommands[] = {"HELLO", "GDBYE", "CREAT", "OPNBX", "NXTMG", "PUTMG", "DELBX", "CLSBX"};
int connected = 0;
char* ipAddress;

int checkCommand(char* str){

	for (int i = 0; i < 7; i++){
		if(strcmp(str,commandOptions[i]) == 0)
			return 1;
	}
	return 0;

}

//char*

char* append(char s[], char c){
	//printf("in append function\n");
	int len = strlen(s);
	//printf("len = %d\n", len);
	char buffer[len + 2];
	strcpy (buffer,s);
	buffer[len] = c;
	//printf("here 1\n");
	buffer[len + 1] = '\0';
	//printf("buffer = %s\n",buffer);
	char* final = malloc(sizeof(buffer));
	strcpy(final,buffer);
	//printf("final = %s\n",final);
	return final;

}

int checkBoxName(char* message){
    int len = strlen(message);
    if (len > 25 || len < 5) return -2;
    char c = message[0];
    if (isalpha(c)) return 0;
    return -1;
}


void handleOpen(int sockfd){
    char message[1024];
    bzero(message,sizeof(message));
    //strcpy(message,clientCommands[3]);
    //write(sockfd, message,sizeof(message)); //SEND OPENBX
    //bzero(message,sizeof(message));
    printf("Okay, open which message box?\n");
    printf("open:> ");
    bzero(message,sizeof(message));

    int c, i = 0;
	char *box = "";

	while((c = getchar()) != EOF ){
		if (c == '\n' && i == 0){
    		 //printf("is first enter key\n");
    		i++;
    		 continue;
    	}
    	if (c == '\n') break;
    	//printf("%c ", c);
    	box = append(box, c);
    	i++;
	}

    //scanf("%s", message);
    int boxLen = strlen(box);
    char boxName[boxLen]; strcpy(boxName,box);
    bzero(message,sizeof(message));
    sprintf(message, "OPNBX %s", boxName);
    //printf("message sending: %s\n", message);

    write(sockfd, message,sizeof(message)); //SEND box name
    //printf("opening box: %s\n", boxName);
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
    //char message[] = "";
    bzero(message,sizeof(message));
    //strcpy(message,clientCommands[2]);
    //write(sockfd, message,sizeof(message)); //SEND CREATEBX
    //bzero(message,sizeof(message));
    printf("Okay, create a box name!\n");
    printf("create:> ");
    //bzero(message,sizeof(message));
    int c, i = 0;
	char *box = "";

	while((c = getchar()) != EOF ){
		if (c == '\n' && i == 0){
    		 //printf("is first enter key\n");
    		i++;
    		 continue;
    	}
    	if (c == '\n') break;
    	//printf("%c ", c);
    	box = append(box, c);
    	i++;
	}

    //scanf("%[^\n]s", message);
    //char* message = scanInput();
    int nameStatus = checkBoxName(box);
    if (nameStatus == -2){ //not acceptable length
	printf("Box name must be 5 to 25 characters long. Please try again.\n");
	return;
    }
    else if (nameStatus == -1){
	printf("Box name must begin with an alphabetical character. Please try again.\n");
	return;
    }

    char boxName[26]; strcpy(boxName,box);
    //int boxLen = strlen(boxName)+1;
    bzero(message,sizeof(message));
    sprintf(message, "CREAT %s", boxName);
    //printf("message sending: %s\n", message);

    write(sockfd, message,sizeof(message)); //SEND box name
    //printf("creating box: %s\n", boxName);
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
    int c, i = 0;
	char *box = "";

	while((c = getchar()) != EOF ){
		if (c == '\n' && i == 0){
    		 //printf("is first enter key\n");
    		i++;
    		 continue;
    	}
    	if (c == '\n') break;
    	//printf("%c ", c);
    	box = append(box, c);
    	i++;
	}
    int boxLen = strlen(box);
    //scanf("%s", message);
    char boxName[boxLen]; strcpy(boxName,box);
    bzero(message,sizeof(message));
    sprintf(message, "CLSBX %s", boxName);
    //printf("message sending: %s\n", message);
    write(sockfd, message,sizeof(message)); //SEND box name
    //printf("closing box: %s\n", boxName);
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
		sent+=send(sockfd, message, size,0);
		//sent+= write(sockfd,message,size);
		//printf("client sent: %d of %d\n", sent, size);
		if (sent < 0) return -1;
	}

	return 0;
}



void handlePut(int sockfd){
    char message[1024];
    printf("Okay, enter your message:\nput:> ");


    int c, i = 0;
	char *msg = "";

	while((c = getchar()) != EOF ){
		if (c == '\n' && i == 0){
    		 printf("is first enter key\n");
    		i++;
    		 continue;
    	}
    	if (c == '\n') break;
    	//printf("%c ", c);
    	msg = append(msg, c);
    	i++;
	}



    //scanf("%s", &msg);
    //printf("PUTMSG = %s\n", msg);

    unsigned int numBytes = strlen(msg);
    //printf("num of bytes: %d\n", numBytes);
    int total = 6+4+1+numBytes+1;
    char* theMesseage = malloc(total);
    sprintf(theMesseage, "PUTMG!%d!%s", numBytes, msg);
    printf("message: %s\n", theMesseage);

    //write(sockfd, message,sizeof(message)); //SEND MSG
    sendMessage(sockfd,theMesseage);
    char weWant[1024]; sprintf(weWant,"OK!%d", numBytes);
    //printf("expecting: %s\n", weWant);
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
    int c, i = 0;
	char *box = "";

	while((c = getchar()) != EOF ){
		if (c == '\n' && i == 0){
    		 printf("is first enter key\n");
    		i++;
    		 continue;
    	}
    	if (c == '\n') break;
    	//printf("%c ", c);
    	box = append(box, c);
    	i++;
	}
    int boxLen = strlen(box);

    //scanf("%s", message);
    char boxName[boxLen]; strcpy(boxName,box);
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

int handleQuit(int sockfd){
    char message[1024];
    bzero(message,sizeof(message));
    strcpy(message,clientCommands[1]);
    write(sockfd, message,sizeof(message)); //SEND GDBYE
    bzero(message,sizeof(message));
    int response = read(sockfd, message, sizeof(message)); printf("READY TO QUIT\n");
    if(response == 0){
        return 1;
    }else{
        return 0;
    }
    //bzero(message,sizeof(message));
    //read(sockfd,message,sizeof(message)); //WAIT FOR MESSAGE
}


void readCommands(int sockfd,char* ipAddress){

	char conn[1024];
	for (;;){
		//bzero(message,sizeof(message));
		if(connected == 0){

            strcpy(conn,clientCommands[0]);
            write(sockfd, conn,sizeof(conn)); //UPON CONNECTION: CLIENT SENDS HELLO
            bzero(conn,sizeof(conn));
	    		read(sockfd,conn,sizeof(conn)); //AFTER RECEIVING READY FROM SERVER, CONTINUE TO RECEIVE MORE COMMANDS
            if(strcmp(conn, "HELLO DUMBv0 ready!") == 0){
                printf("HELLO DUMBv0 ready!\n");
                connected = 1;
            }else{
                printf("Connection failed\n");
                break;
            }
		}
		printf("> ");
		//bzero(message,sizeof(message));
		
		int c, i = 0;
		char *message = "";

		while((c = getchar()) != EOF ){
		if (c == '\n' && i == 0){
    		 //printf("is first enter key\n");
    		i++;
    		 continue;
    	}
    	if (c == '\n') break;
    	//printf("%c ", c);
    	message = append(message, c);
    	i++;
		}


		//char* message = scanInput();

		//printf("message is: %s\n", message);
		if (strcmp(message, "quit") == 0){
            int response = handleQuit(sockfd);
            if(response == 1) break;
            else {
                printf("error disconnecting\n");
                continue;
            }
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
		
		//else, send the command to server to print ER:WHAT?
			
		printf("This is not a valid command. Try again!\n"); 
		write(sockfd,message,strlen(message));
		continue;
		
		
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
	while(numAttempts < 3){
		if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
	        printf("connecting failed :(\n");
	        exit(0);
		numAttempts ++;
		continue;
    		}
		else break;
	}


    //get ip address to pass to socket
    char str  [1024];
    str[1023] = '\0';
    gethostname(str, 1023);
    printf("curr machine host: %s\n", str);
    struct hostent *currHost = gethostbyname(str);
    ipAddress  = inet_ntoa(*((struct in_addr*) currHost->h_addr_list[0]));
    char ip[20];
    strcpy(ip,ipAddress); //SEND THE IP ADDRESS
    printf("ip addy of this machine: %s\n", ip);
    write(sockfd,ip,sizeof(ip));


    readCommands(sockfd,ipAddress);

    close(sockfd);
}

