#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "queue.h"
#include <pthread.h>

typedef struct box{
    char* name; //NAME OF BOX
    struct Node* queue; //THE QUEUE
    struct box* next; //POINTER TO NEXT BOX
    int inUse; //BOX STATUS
    int isLocked;
    pthread_mutex_t lock;
}box;


typedef struct tNode{
	pthread_t tid;
	struct tNode* next;

} tNode;

struct tArgs{
	int connfd;
	pthread_t tid;
} ;

//server socket port number: random number between 4096 and 65k
struct box* head = NULL;
char* clientCommands[] = {"HELLO", "GDBYE", "CREAT ", "OPNBX ", "NXTMG", "PUTMG!", "DELBX ", "CLSBX "};
tNode* tHead = NULL;

void deleteNode(tNode* tHead){


}

int createBox(char* name){
    printf("createbox intiated\n");
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
    newBox->isLocked = 0;
    struct Node* qHead = (struct Node*) malloc(sizeof(struct Node));
    newBox->queue = qHead;

    newBox->next = head;
    head = newBox;
    printBox();
    sleep(10);
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
    if (ptr->inUse == 1) //already in use...
	return 0;
    ptr->inUse = 1;
    //check if locked:
    if (ptr->isLocked == 1) {
	printf("already locked!! sorry\n");
	return 0;
    }
   
    //lock the box
    if (pthread_mutex_init(&ptr->lock, NULL) != 0){ 
	printf("mutex failed\n");
	return 0;
    }
    int lock_status = pthread_mutex_lock(&ptr->lock);
    if (lock_status < 0) return 0; //there was an error locking
    ptr->isLocked == 1;
    printf("box \"%s\" has been locked!!!\n", ptr->name);
    //TODO when to unlock??
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
    if(ptr->inUse == 0) //already closed
	return 0;
    if (ptr->isLocked == 0) //not locked...could be problem
	return 0;
    	
    ptr->inUse = 0;
    //actually unlock the box
    int unlock_status = pthread_mutex_unlock(&ptr->lock);
    if (unlock_status < 0) return 0; //error unlocking
    ptr->isLocked = 0;
            

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

int openCommands(char* name, int connfd, struct tArgs* arg){
    //TODO: WAIT HERE FOR OTHER COMMANDS?: NEXT, PUT, CLOSE

    char message[1024];
    for(;;){
        bzero(message,sizeof(message));
        //read(connfd,message,sizeof(message));
        recv(connfd, message, 1024,0);
        printf("client sent message: %s\n", message);
        if (strcmp(message, "exit") == 0) break;
        if (strcmp(message, clientCommands[1]) == 0){ // GDBYE FROM CLIENT
            int status = closeBox(name, name);
            if(status == 1){
                printf("box closed\n");
            }
            arg->tid = 0;
            pthread_exit(NULL);
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
            printf("The message: %s\n", message);
            //get the actual message
            char com[10];
            int bytes;
            char m[1024];
            sscanf(message, "%5s!%d!%s", com, &bytes, m);
            printf("com: %s\n", com);
            printf("len: %d\n", bytes);
            printf("m: %s\n", m);
            int gotLen = strlen(m);
            char theRest[bytes+1]; bzero(theRest, sizeof(theRest));
            char* finalMsg = malloc(bytes+1);
            if(gotLen < bytes){
                printf("have to read again\n");
                recv(connfd, theRest, bytes+1,0);
                printf("the rest:%s\n", theRest);
            }
            sprintf(finalMsg, "%s%s", m, theRest);
            printf("final msg: %s\n", finalMsg);
            int status = putMessage(name, finalMsg); //putmessage
            bzero(message,sizeof(message));
            if(status == 0){
                strcpy(message,"ER:NOOPN");
            }else{
                sprintf(message, "OK!%d", bytes);
            }
            printf("PUTMG!%d!%s\n", bytes, finalMsg);
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
        if (strncmp(message, clientCommands[2], 6) == 0){ //CREAT FROM CLIENT
            printf("time to create!!\n");
            //bzero(message,sizeof(message));
            //read(connfd,message,sizeof(message)); //WAIT FOR BOX NAME
            char boxName[1024]; strcpy(boxName,message+6);
            printf("Create box %s\n", boxName);
            //CREATE BOX AND RETURN STATUS

            /// TODO: HAVE TO CHECK FOR ACCEPTABLE BOX NAMES
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
		if (strncmp(message, clientCommands[3], 6) == 0){ //OPNBX FROM CLIENT
            printf("time to open!!\n");
            //bzero(message,sizeof(message));
            //read(connfd,message,sizeof(message)); //WAIT FOR BOX NAME
            char boxName[1024]; strcpy(boxName,message+6);
            printf("Open box %s\n", boxName);
            strcpy(message,"ER:WHAT?");
            printf("OPNBX %s\n", boxName);
            printf("%s\n", message);
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

void* interpretCommands(void* connfdPtr){
        struct tArgs* arg = (struct tArgs*) connfdPtr;
        int connfd = arg->connfd;
        int tID = arg->tid;
        //int connfd = *((int*) connfdPtr);

        pthread_mutex_t comm_mutex;
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
		if (strcmp(message, clientCommands[1]) == 0){ //CLIENT SAID GDBYE
            arg->tid = 0;
            pthread_exit(NULL);
            break; ///TODO: might have more to do for quit
		}
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
                status = openCommands(boxName, connfd, arg);
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
            int lockStatus = pthread_mutex_lock(&comm_mutex);
            printf("lock status: %d\n");
            int status = createBox(boxName);
            lockStatus = pthread_mutex_unlock(&comm_mutex);
            printf("lock status: %d\n");
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
		if (strncmp(message, clientCommands[5], 6) == 0){ //PUTMG
            char msg[1024]; strcpy(msg,message+6);
            bzero(message,sizeof(message));
            strcpy(message,"ER:NOOPN");
            printf("PUTMG!%s\n", msg);
            printf("%s\n", message);
            write(connfd, message,sizeof(message));
            continue;
		}
		if (strncmp(message, clientCommands[4], 5) == 0){ //NXTMG
            //char boxName[1024]; strcpy(boxName,message+6);
            bzero(message,sizeof(message));
            strcpy(message,"ER:NOOPN");
            printf("NXTMG\n");
            printf("%s\n", message);
            write(connfd, message,sizeof(message));
            continue;
		}
		if (strncmp(message, clientCommands[7], 6) == 0){ //CLSBX
            char boxName[1024]; strcpy(boxName,message+6);
            bzero(message,sizeof(message));
            strcpy(message,"ER:NOOPN");
            printf("CLSBX %s\n", boxName);
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
/*
void* acceptConn(void* args){
	    struct connArgs cArgs = *((struct connArgs*) args);
	    int sockfd = cArgs.sockfd;
	    int size = cArgs.sockfd;
            struct sockaddr_in clientaddr = cArgs.clientaddr;
	    pthread_t tid = cArgs.tid;

	    int connfd = accept(sockfd, (struct sockaddr*)&clientaddr, &size);
	    if (connfd < 0) {
        	printf("could not accept client :(\n");
	        return 0;
	    }
	    printf("connected\n");


	    printf("adding thread to LL...\n");
	    if (tHead == NULL){
		tHead = (tNode*) malloc(sizeof(tNode));
		tHead->next = NULL;
		tHead->tid = tid;
	    }
	    else{
		tNode* new = (tNode*) malloc(sizeof(tNode));
		new->next = tHead;
		new->tid = tid;
		tHead=new;
	    }



            interpretCommands((void*) &connfd);


}
*/
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
    //start a thread here
    pthread_t tid;


    //tNode* tHead = malloc(sizeof(tNode));


    // Accept the data packet from client and verification
    while( (connfd = accept(sockfd, (struct sockaddr*)&clientaddr, &size))){
    /*
	        struct connArgs* cArgs = malloc(sizeof(struct connArgs));
 		cArgs->size = size;
    		cArgs->clientaddr = clientaddr;
		cArgs->sockfd = sockfd;
   	*/
   	printf("connection accepted\n");
	      //threading
	      struct tArgs* arg = malloc(sizeof(struct tArgs));
	      arg->connfd = connfd;
	      arg->tid = tid;
	   if (pthread_create(&tid, NULL, interpretCommands,(void*) arg) < 0){
	   	printf("could not thread :( \n");
		return 0;
	   }
 	   printf("handler assigned\n");
 	   pthread_detach(tid);
 	   //ADD TO THE LL
 	   printf("adding thread to LL...\n");
	    if (tHead == NULL){
		tHead = (tNode*) malloc(sizeof(tNode));
		tHead->next = NULL;
		tHead->tid = tid;
	    }
	    else{
		tNode* new = (tNode*) malloc(sizeof(tNode));
		new->next = tHead;
		new->tid = tid;
		tHead=new;
	    }


 	   //join
 	   printf("here we join?\n");
 	   tNode* ptr = tHead;
        //while(ptr!=NULL){
		//pthread_join(ptr->tid, NULL);
		//printf("joined!!\n");
	    //}


   //interpretCommands(connfd);

   //join threads here
    }

    if (connfd < 0)
    {
        printf("accept failed");
        return 1;
    }



  close(sockfd);

}

