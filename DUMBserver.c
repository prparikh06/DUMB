#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "queue.h"

typedef struct box{
    char* name; //NAME OF BOX
    struct Node* queue; //THE QUEUE
    struct box* next; //POINTER TO NEXT BOX
    int inUse; //BOX STATUS
}box;

//server socket port number: random number between 4096 and 65k
struct box* head = NULL;
char* clientCommands[] = {"HELLO", "GDBYE", "CREAT ", "OPNBX ", "NXTMG", "PUTMG!", "DELBX ", "CLSBX "};


int createBox(char* name){
    box* ptr = head;

    while(ptr != NULL){ //if head hasn't been initialized
        if(strcmp(ptr->name, name) == 0){
            return 0; //DUPLICATE
        }
        ptr = ptr->next;
    }

    box* newBox = malloc(sizeof(box));
    if(!newBox){
        printf("could not create box\n");
        return 0;
    }
    newBox->name = malloc(sizeof(char)*1024);
    strcpy(newBox->name,name);
    newBox->inUse = 0;
    struct Node* qHead = (struct Node*) malloc(sizeof(struct Node));
    newBox->queue = qHead;

    newBox->next = head;
    head = newBox;
    printBox();
    return 1;
}

int openBox(char* name){
    box* ptr = head;
    int found = 0;

    while(ptr != NULL){
        printf("currbox name: %s, newname: %s\n", ptr->name, name);
        if(strcmp(ptr->name, name) == 0){
            found = 1;
            break;//FOUND BOX
        }
        ptr = ptr->next;
    }
    if(found == 0){
        return 0; //box doesn't exist
    }
    ptr->inUse = 1;
    return 1;
    //TODO: WAIT HERE FOR OTHER COMMANDS?: NEXT, PUT, CLOSE

}
void printBox(){
    box* ptr = head;

    while(ptr != NULL){
        printf("%s\t", ptr->name);
        ptr = ptr->next;
    }
    printf("\n");
}
int deleteBox(char* name){
    box* ptr = head;
    box* prev = head;
    int found = 0;
    printBox();
    while(ptr != NULL){
        printf("currbox name: %s, newname: %s\n", ptr->name, name);
        if(strcmp(ptr->name, name) == 0){
            found = 1;
            break;//FOUND BOX
        }
        prev = ptr;
        ptr = ptr->next;
    }
    if(found == 0){
        return 0; //box doesn't exist
    }
    if(ptr->inUse == 1) return -1; //CURRENTLY OPEN, CANNOT DELETE
    if(ptr->queue->data != NULL) return -2; //BOX NOT EMPTY, CANNOT DELETE

    printf("head is:%s\n", head->name);
    printf("prev is:%s\n", prev->name);

    if(ptr == prev) head = ptr->next; //its the first element
    //else if(prev == head) head = prev->next;
    else prev->next = ptr->next;
    if(head != NULL){
        printf("head isnow :%s\n", head->name);
    }



    free(ptr->name);
    free(ptr->queue);
    free(ptr);
    printBox();
    return 1;


}

int closeBox(char* name, char* target){
    if(strcmp(name,target) != 0){ //Current open box does not match closebox arg
        return 0;
    }
    box* ptr = head;

    while(ptr != NULL){
        if(strcmp(ptr->name, target) == 0){
            break;//FOUND BOX
        }
        ptr = ptr->next;
    }

    ptr->inUse = 0;
    return 1;

}

int putMessage(char* name, char* msg){
    printf("my message: %s\n", msg);
    box* ptr = head;

    while(ptr != NULL){
        if(strcmp(ptr->name, name) == 0){
            break;//FOUND BOX
        }
        ptr = ptr->next;
    }
    if(ptr->inUse == 0) return 0;
    //struct Node* q = ptr->queue;
    enqueue(&ptr->queue, msg);
    printList(ptr->queue);
    printf("in my box: %s\n", ptr->queue->data);
    return 1;

}

char* getNextMsg(char* name){
    box* ptr = head;

    while(ptr != NULL){
        if(strcmp(ptr->name, name) == 0){
            break;//FOUND BOX
        }
        ptr = ptr->next;
    }
    if(ptr->inUse == 0) return "ER:NOOPN";
    //struct Node* q = ptr->queue;
    if(ptr->queue == NULL){
        return "ER:EMPTY"; //List is emptyyy
    }

    char* msg  = dequeue(&ptr->queue);
    printf("message is %s\n", msg);

    return msg;

}

int convertNum(char* num){
    char c;
    int i, digit, number = 0;
    for(i=0; i<strlen(num);i++){
        c = num[i];
        if(c>= '0' && c<= '9'){
            digit = c - '0';
            number = number * 10 + digit;
        }
    }
    return number;
}

int openCommands(char* name, int connfd){
    //TODO: WAIT HERE FOR OTHER COMMANDS?: NEXT, PUT, CLOSE

    char message[1024];
    for(;;){
        bzero(message,sizeof(message));
        read(connfd,message,sizeof(message));
        printf("client sent message: %s\n", message);
        if (strcmp(message, "exit") == 0) break;
        if (strcmp(message, clientCommands[1]) == 0){ // GDBYE FROM CLIENT
            int status = closeBox(name, name);
            if(status == 1){
                printf("box closed\n");
            }
            return 0; //RETURNING 0 TO INDICATE CLOSE CONNECTION
        }
        if (strncmp(message, clientCommands[7], 6) == 0){ //CLSBX FROM CLIENT
            printf("time to close!!\n");
            //bzero(message,sizeof(message));
            //read(connfd,message,sizeof(message)); //WAIT FOR BOX NAME
            char boxName[1024]; strcpy(boxName,message+6);

            printf("Close box %s\n", boxName);
            int status = closeBox(name, boxName);
            bzero(message,sizeof(message));
            if(status == 0){
                strcpy(message,"ER:NOOPN");
            }else{
                strcpy(message,"OK!");
            }
            printf("CLSBX %s\n", boxName);
            printf("%s\n", message);
            write(connfd, message,sizeof(message));
            if(status == 1) return 1; //SUCCESSFULLY CLOSED BOX
            continue;
        }
        if (strncmp(message, clientCommands[5], 6) == 0){ //RECEIVED PUTMG
            printf("time to put a msg!!\n");
            //bzero(message,sizeof(message));
            //read(connfd,message,sizeof(message)); //WAIT FOR MSG
            printf("The message: %s\n", message);
            char msg[1024]; bzero(msg,sizeof(msg));
            //get the actual message
            char* numBytes = strtok(message, "!");
            numBytes = strtok(NULL, "!");
            int bytes = convertNum(numBytes);

            int len = strlen(numBytes); //index where actual msg starts
            len = 6 + len + 1;
            strcpy(msg,message+len);
            int status = putMessage(name, msg); //putmessage
            bzero(message,sizeof(message));
            if(status == 0){
                strcpy(message,"ER:NOOPN");
            }else{
                sprintf(message, "OK!%d", bytes);
            }
            printf("PUTMG!%d!%s\n", bytes, msg);
            printf("%s\n", message);
            write(connfd, message,sizeof(message));
            continue;
        }
        if (strcmp(message, clientCommands[4]) == 0){ //NXTMG
            bzero(message,sizeof(message));
            char* msg = getNextMsg(name);
            printf("nextmesg returned %s\n", msg);
            int len = strlen(msg)+1;
            char* nextMsg = malloc(len);
            nextMsg = msg;

            if(strcmp(nextMsg, "ER:EMPTY") == 0){
                strcpy(message, "ER:EMPTY");
            }
            else if(strcmp(nextMsg, "ER:NOOPN") == 0){
                strcpy(message, "ER:EMPTY");
            }
            else{
                int bytes = strlen(msg)+1;
                sprintf(message,"OK!%d!%s", bytes, msg);
            }

            printf("%s\n", message);
            write(connfd, message,sizeof(message));
            continue;
        }
        if (strncmp(message, clientCommands[6], 6) == 0){ //DELETEBX
            bzero(message,sizeof(message));
            strcpy(message, "ER:OPEND");    //CANNOT DELETE OPENED BOX
            printf("message is: %s\n", message);
            write(connfd, message,sizeof(message));
            continue;
        }

        bzero(message,sizeof(message));
        printf("Enter a message...\n");
        scanf("%s", message);
        printf("message is: %s\n", message);
        write(connfd, message,sizeof(message));
        if (strcmp(message, "exit") == 0) break;
    }

}

void interpretCommands(int connfd){
	char message[1024];
	for (;;){
		bzero(message,sizeof(message));
		read(connfd,message,sizeof(message));
		printf("client sent message: %s\n", message);
		if (strcmp(message, clientCommands[0]) == 0){ //HELLO FROM CLIENT
            bzero(message,sizeof(message));
            strcpy(message,"HELLO DUMBv0 ready!"); // AFTER RECEIVING HELLO, SIGNAL READY
            write(connfd, message,sizeof(message));
            printf("HELLO\n"); //add timestamps laterrr
            continue;
		}
		if (strcmp(message, clientCommands[1]) == 0) break;
		if (strncmp(message, clientCommands[3], 6) == 0){ //OPNBX FROM CLIENT
            printf("time to open!!\n");
            //bzero(message,sizeof(message));
            //read(connfd,message,sizeof(message)); //WAIT FOR BOX NAME
            char boxName[1024]; strcpy(boxName,message+6);
            printf("Open box %s\n", boxName);

            int status = openBox(boxName);
            bzero(message,sizeof(message));
            if(status == 0){
                strcpy(message,"ER:NEXST");
            }else{
                strcpy(message,"OK!");
            }
            printf("OPNBX %s\n", boxName);
            printf("%s\n", message);
            write(connfd, message,sizeof(message));
            //TODO: Listen for openned box commands
            if(status == 1){
                status = openCommands(boxName, connfd);
                if(status == 0){ //CLIENT CLOSED CONNECTION
                    break;
                }
            }// OTHERWISE, BOX CLOSED, CONTINUE LISTENING
            continue;
		}
		if (strncmp(message, clientCommands[2], 6) == 0){ //CREAT FROM CLIENT
            printf("time to create!!\n");
            //bzero(message,sizeof(message));
            //read(connfd,message,sizeof(message)); //WAIT FOR BOX NAME
            char boxName[1024]; strcpy(boxName,message+6);
            printf("Create box %s\n", boxName);
            //CREATE BOX AND RETURN STATUS

            // TODO: HAVE TO CHECK FOR ACCEPTABLE BOX NAMES
            int status = createBox(boxName);
            bzero(message,sizeof(message));
            if(status == 0){
                strcpy(message,"ER:EXIST");
            }else{
                strcpy(message,"OK!");
            }
            printf("CREAT %s\n", boxName);
            printf("%s\n", message);
            write(connfd, message,sizeof(message));

            continue;
		}
		if (strncmp(message, clientCommands[6], 6) == 0){ //DELETEBX
            printf("time to DELETE!!\n");
            //bzero(message,sizeof(message));
            //read(connfd,message,sizeof(message)); //WAIT FOR BOX NAME
            char boxName[1024]; strcpy(boxName,message+6);
            printf("Delete box %s\n", boxName);

            int status = deleteBox(boxName);
            bzero(message,sizeof(message));
            if(status == 0){
                strcpy(message,"ER:NEXST");
            }else if(status == -1){
                strcpy(message,"ER:OPEND");
            }else if(status == -2){
                strcpy(message,"ER:NOTMT");
            }else{
                strcpy(message,"OK!");
            }
            printf("DELBX %s\n", boxName);
            printf("%s\n", message);
            write(connfd, message,sizeof(message));

            continue;

		}
		if (strcmp(message, "exit") == 0) break;
		bzero(message,sizeof(message));
		printf("Enter a message...\n");
		scanf("%s", message);
		printf("message is: %s\n", message);
		write(connfd, message,sizeof(message));
		if (strcmp(message,"exit") == 0) break;


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
    printf("connected\n");



   interpretCommands(connfd);

   //join threads here

   close(sockfd);

}

