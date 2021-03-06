#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "queue.h"
#include <pthread.h>
#include <time.h>
#include <math.h>

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
	char ip[20];
} ;

//server socket port number: random number between 4096 and 65k
struct box* head = NULL;
char* clientCommands[] = {"HELLO", "GDBYE", "CREAT ", "OPNBX ", "NXTMG", "PUTMG!", "DELBX ", "CLSBX "};
tNode* tHead = NULL;
pthread_mutex_t globalLock;

char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec"};

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

char* getTime(){

	time_t t = time(NULL);
	int addZero = 0;
	struct tm tm = *localtime(&t);

	int hour =  tm.tm_hour;
	int military = hour* 100 + tm.tm_min;
	if (hour == 0) //midnight
		addZero = 2;
	else if (hour < 10) addZero = 1;
	char* month = months[tm.tm_mon];
	int date = tm.tm_mday;
	int len = snprintf(NULL,0,"%d", date);
	char* day = (char*) malloc(sizeof(char) * (len + 1));
	sprintf(day, "%d",date);
	//printf("day = %s\n", day);
	char* ddMM = (char*) malloc(strlen(day) + 1 + strlen(month) + 1);//[50];
	strcat(ddMM, day);

	strcat(ddMM, " ");
	strcat(ddMM, month);
	//printf("ddMM = %s\n", ddMM);

	char* zero = (char*) malloc(3);
	if (addZero == 2) strcat(zero, "00");
	else if (addZero == 1) strcat(zero,"0");
	else strcat(zero, "");
	//printf("zero = %s\n", zero);

	len = snprintf(NULL, 0,"%d", military);
	char* time = (char*) malloc(sizeof(char) * (len + 1));
	sprintf(time, "%d", military);
	char* output = (char*) malloc(strlen(zero) + strlen(time) + strlen(ddMM) + 2);

	strcat(output,zero);
	strcat(output,time);
	//printf("output = %s\n", output);
	strcat(output," ");
	//printf("output = %s\n", output);
	strcat(output,ddMM);
	//printf("final output: %s\n",output);
	//char* timestamp = malloc(strlen(output) + 1);
	//strcpy(timestamp,output);
	free(day); free(zero);free(time);free(ddMM);
	return output;

}


void eventOutput(char* ip, char* event){
	//printf("ip = %s, even = %s\n", ip,event);

	char* time = getTime();
	char* output = (char*) malloc(strlen(time) + 1 + strlen(ip) + 1 + strlen(event) + 1);
	strcat(output, time);
	strcat(output, " ");
	strcat(output, ip);
	strcat(output, " ");
	strcat(output, event);
	printf("%s\n", output);
	free(output);
}


void deleteNode(tNode** tHead, pthread_t target){
	tNode* ptr = *tHead;
	tNode* prev = ptr;
	while(ptr != NULL){
		if (ptr->tid == target){ //found tid
			prev->next = ptr->next;
			free(ptr);
			return;
		}
		prev = ptr;
		ptr = ptr->next;
	}
	return;

}


int createBox(char* name){

    //lock tid
    if (pthread_mutex_init(&globalLock, NULL) < 0){
	//printf("mutex failed\n");
	return 0;
    }

    int lock_status = pthread_mutex_lock(&globalLock);
    if (lock_status < 0){
	//printf("error locking\n");
	return 0;
    }

    //printf("createbox intiated\n");
    box* ptr = head;

    while(ptr != NULL){ //if head hasn't been initialized
        if(strcmp(ptr->name, name) == 0){
            return 0; //DUPLICATE
        }
        ptr = ptr->next;
    }

    //check box name
    int nameStatus = checkBoxName(name);
    if (nameStatus == -2){ //not acceptable length
	    //printf("Box name must be 5 to 25 characters long. Please try again.\n");
	    return -2;
    }
    else if (nameStatus == -1){
	    //printf("Box name must begin with an alphabetical character. Please try again.\n");
	    return -1;
    }


    box* newBox = malloc(sizeof(box));
    if(!newBox){
        //printf("could not create box\n");
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
    //printBox();
    //sleep(10);
    //printf("sleeping...\n");
    //unlock
    int unlock_status = pthread_mutex_unlock(&globalLock);
    if (unlock_status < 0) {
	//printf("error unlocking\n");
	return 0;
    }
    return 1;
}

int openBox(char* name){
    box* ptr = head;
    int found = 0;

    while(ptr != NULL){
        //printf("currbox name: %s, newname: %s\n", ptr->name, name);
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
	return -1;

    //check if locked:
    if (ptr->isLocked == 1) {
	//printf("already locked!! sorry\n");
	return -1;
    }

    //lock the box
    if (pthread_mutex_init(&ptr->lock, NULL) < 0){
	//printf("mutex failed\n");
	return 0;
    }
    int lock_status = pthread_mutex_lock(&ptr->lock);
    if (lock_status < 0) return 0; //there was an error locking
    ptr->isLocked = 1;
    //printf("box \"%s\" has been locked!!!\n", ptr->name);

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
    //printBox();
    while(ptr != NULL){
        //printf("currbox name: %s, newname: %s\n", ptr->name, name);
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

    //printf("head is:%s\n", head->name);
    //printf("prev is:%s\n", prev->name);

    if(ptr == prev) head = ptr->next; //its the first element
    //else if(prev == head) head = prev->next;
    else prev->next = ptr->next;
    /*
    if(head != NULL){
        printf("head isnow :%s\n", head->name);
    }
	*/


    free(ptr->name);
    free(ptr->queue);
    free(ptr);
    //printBox();
    return 1;


}

int checkBoxName(char* message){
    int len = strlen(message);
    if (len > 25 || len < 5) return -2;
    char c = message[0];
    if (isalpha(c)) return 0;
    return -1;
}

int closeBox(char* name, char* target){
    if(strcmp(name,target) != 0){ //Cnot evenopen box does not match closebox arg
        return 0;
    }
    box* ptr = head;

    while(ptr != NULL){
        if(strcmp(ptr->name, target) == 0){
            break;//FOUND BOX
        }
        ptr = ptr->next;
    }
    if(ptr->inUse == 0){ //already closed
	//printf("box is not even in use :(\n");
	return 0;
    }
    //printf("HERE IN CLOSING\n");
   /* if (ptr->isLocked == 0){ //not locked...could be problem
	printf("box is not even locked..??\n");
	return 0;
    }
*/
    ptr->inUse = 0;
    //actually unlock the box
    int unlock_status = pthread_mutex_unlock(&ptr->lock);
    //printf("unlock status = %d\n", unlock_status);
    if (unlock_status < 0) return 0; //error unlocking
    ptr->isLocked = 0;


    return 1;

}

int putMessage(char* name, char* msg){
    //printf("my message: %s\n", msg);

    int lock_status = pthread_mutex_lock(&globalLock);
    if (lock_status < 0){
	printf("error locking\n");
	return 0;
    }

    box* ptr = head;

    while(ptr != NULL){
        if(strcmp(ptr->name, name) == 0){
            break;//FOUND BOX
        }
        ptr = ptr->next;
    }

    //sleep(10);
    int unlock_status = pthread_mutex_unlock(&globalLock);
    if (unlock_status < 0) {
	printf("error unlocking\n");
	return 0;
    }

    if(ptr->inUse == 0) return 0;
    //struct Node* q = ptr->queue;
    enqueue(&ptr->queue, msg);
    //printList(ptr->queue);
    //printf("in my box: %s\n", ptr->queue->data);
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
    if(ptr->inUse == 0){

    	 return "ER:NOOPN";
    }
    //struct Node* q = ptr->queue;
    if(ptr->queue == NULL){
        return "ER:EMPTY"; //List is emptyyy
    }

    char* msg  = dequeue(&ptr->queue);
    //printf("message is %s\n", msg);

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
    char* ip = arg->ip;
    char message[1024];
    for(;;){
        bzero(message,sizeof(message));
        //read(connfd,message,sizeof(message));
        recv(connfd, message, 1024,0);
        //printf("client sent message: %s\n", message);
        if (strcmp(message, "exit") == 0) break;
        if (strcmp(message, clientCommands[1]) == 0){ // GDBYE FROM CLIENT
            int status = closeBox(name, name);
            if(status == 1){
                eventOutput(ip, "CLSBX");
                //printf("box closed\n");
            }
	    deleteNode(&tHead,arg);
            //arg->tid = 0;
            close(connfd);
	    		//event output: GDBYE and disconnect
	    		eventOutput(ip, "GDBYE");
	    		eventOutput(ip,"disconnected");
            pthread_exit(NULL);
            return 0; //RETURNING 0 TO INDICATE CLOSE CONNECTION
        }
        if (strncmp(message, clientCommands[7], 6) == 0){ //CLSBX FROM CLIENT
            //printf("time to close!!\n");
            //bzero(message,sizeof(message));
            //read(connfd,message,sizeof(message)); //WAIT FOR BOX NAME
            char boxName[1024]; strcpy(boxName,message+6);

            //printf("Close box %s\n", boxName);
            int status = closeBox(name, boxName);
            bzero(message,sizeof(message));
            if(status == 0){
            	//error output:NOOPN
    				eventOutput(ip,"ER:NOOPN");
                strcpy(message,"ER:NOOPN");
            }else{
                strcpy(message,"OK!");
					//event output: CLSBX
	    			eventOutput(ip, "CLSBX");

            }
            //printf("CLSBX %s\n", boxName);
	  			//printf("%s\n", message);
            write(connfd, message,sizeof(message));
            if(status == 1) return 1; //SUCCESSFULLY CLOSED BOX
            continue;
        }

        if (strncmp(message, clientCommands[5], 6) == 0){ //RECEIVED PUTMG
            //printf("time to put a msg!!\n");
            //printf("The message: %s\n", message);
            //get the actual message
            char com[10];
            int bytes;
            char m[1024];
            sscanf(message, "%5s!%d!%[^\n]", com, &bytes, m);
            //printf("com: %s\n", com);
            //printf("len: %d\n", bytes);
            //printf("m: %s\n", m);
            int gotLen = strlen(m);
            char theRest[bytes+1]; bzero(theRest, sizeof(theRest));
            char* finalMsg = malloc(bytes+1);
            if(gotLen < bytes){
                //printf("have to read again\n");
                recv(connfd, theRest, bytes+1,0);
                //printf("the rest:%s\n", theRest);
            }
            sprintf(finalMsg, "%s%s", m, theRest);
            //printf("final msg: %s\n", finalMsg);
            int status = putMessage(name, finalMsg); //putmessage
            bzero(message,sizeof(message));
            if(status == 0){
                //error output:NOOPN
    			eventOutput(ip,"ER:NOOPN");
                strcpy(message,"ER:NOOPN");
            }else{
                //event output:PUTMG
    			eventOutput(ip,"PUTMG");
                sprintf(message, "OK!%d", bytes);
            }
            //printf("PUTMG!%d!%s\n", bytes, finalMsg);
            //printf("%s\n", message);
            write(connfd, message,sizeof(message));
            continue;
        }

        if (strcmp(message, clientCommands[4]) == 0){ //NXTMG
            bzero(message,sizeof(message));

            char* msg = getNextMsg(name);
            //printf("nextmesg returned %s\n", msg);
            int len = strlen(msg);
            char* nextMsg = malloc(len+1);
            nextMsg = msg;
            //char* messageToSend = malloc(3+5+len);

            //char* nextMsg = malloc(len);
            //nextMsg = msg;


            if(strcmp(nextMsg, "ER:EMPTY") == 0){
            	//error output:EMPTY
    				eventOutput(ip,"ER:EMPTY");
                strcpy(message, "ER:EMPTY");
                 write(connfd, message,sizeof(message));
                
		continue;
            }
            else if(strcmp(nextMsg, "ER:NOOPN") == 0){
            	//error output:NOOPN
    				eventOutput(ip,"ER:NOOPN");
                strcpy(message, "ER:NOOPN");
 		 write(connfd, message,sizeof(message));
                               continue;
            }
            else{

                int bytes = strlen(msg);
                sprintf(message,"OK!%d!%s", bytes, msg);


                //printf("%s\n", message);
                write(connfd, message,sizeof(message));
                //sent OK, now send the actual size of message and message itself
                int converted = htonl(bytes);
                write(connfd, &converted, sizeof(converted));
                write(connfd, nextMsg, strlen(nextMsg));
                //event output: NXTMG
                eventOutput(ip,"NXTMG");
                continue;
            }


        }
        if (strncmp(message, clientCommands[6], 6) == 0){ //DELETEBX
            bzero(message,sizeof(message));
            strcpy(message, "ER:OPEND");    //CANNOT DELETE OPENED BOX
            //error output:OPEND
    				eventOutput(ip,"ER:OPEND");
            //printf("message is: %s\n", message);
            write(connfd, message,sizeof(message));
            continue;
        }
        if (strncmp(message, clientCommands[2], 6) == 0){ //CREAT FROM CLIENT
            //printf("time to create!!\n");
            //bzero(message,sizeof(message));
            //read(connfd,message,sizeof(message)); //WAIT FOR BOX NAME
            char boxName[1024]; strcpy(boxName,message+6);
            //printf("Create box %s\n", boxName);
            //CREATE BOX AND RETURN STATUS


            int status = createBox(boxName);
            //printf("box status %d\n", status);
            bzero(message,sizeof(message));
            if (status == -1 || status == -2) {
                //error output: WHAT - should be between 5 and 25 characters and start with alphabetical char
                eventOutput(ip, "ER:WHAT");
                strcpy(message,"ER:WHAT");
            }
            else if(status == 0){
            	//error output:EXIST
    				eventOutput(ip,"ER:EXIST");
                strcpy(message,"ER:EXIST");
            }else{
                strcpy(message,"OK!");
				//event output: CREAT
				eventOutput(ip,"CREAT");
            }
            //printf("CREAT %s\n", boxName);
            //printf("%s\n", message);
            write(connfd, message,sizeof(message));

            continue;
		}
		if (strncmp(message, clientCommands[3], 6) == 0){ //OPNBX FROM CLIENT
		/*
		*
		*		THIS IS THE ERROR! (extra credit) - trying to open a box from a currently opened box!
		*
		*/
            //printf("time to open!!\n");
            //bzero(message,sizeof(message));
            //read(connfd,message,sizeof(message)); //WAIT FOR BOX NAME
            char boxName[1024]; strcpy(boxName,message+6);
            //printf("Open box %s\n", boxName);
            //error output:ALOPN
    				eventOutput(ip,"ER:ALOPN");
            strcpy(message,"ER:ALOPN");
            //printf("OPNBX %s\n", boxName);
            //printf("%s\n", message);
            write(connfd, message,sizeof(message));
            continue;
        }



			//if none of the above, then ER:WHAT?
			//event output: WHAT?
			eventOutput(ip,"ER:WHAT?");
			continue;

    }

}

void* interpretCommands(void* connfdPtr){
        struct tArgs* arg = (struct tArgs*) connfdPtr;
        int connfd = arg->connfd;
        int tID = arg->tid;
        char *ip = arg->ip;
        //printf("interpreting commands' ip addy: %s\n", ip);
        //int connfd = *((int*) connfdPtr);
	//detach here
	pthread_detach(pthread_self());


        pthread_mutex_t comm_mutex;
	char message[1024];
	for (;;){
		bzero(message,sizeof(message));
		read(connfd,message,sizeof(message));
		//printf("client sent message: %s\n", message);
		if (strcmp(message, clientCommands[0]) == 0){ //HELLO FROM CLIENT
            bzero(message,sizeof(message));
            strcpy(message,"HELLO DUMBv0 ready!"); // AFTER RECEIVING HELLO, SIGNAL READY
            write(connfd, message,sizeof(message));
            //printf("HELLO\n"); //add timestamps laterrr
            //event output: HELLO
            eventOutput(ip,"HELLO");
	    continue;
		}
		if (strcmp(message, clientCommands[1]) == 0){ //CLIENT SAID GDBYE
            	deleteNode(&tHead, arg);
		//arg->tid = 0;
             close(connfd);
				//event output; GDBYE and disconnected
				eventOutput(ip,"GDBYE");
			eventOutput(ip,"disconnected");
            pthread_exit(NULL);

            break; ///TODO: might have more to do for quit/disconnect
		}
		if (strncmp(message, clientCommands[3], 6) == 0){ //OPNBX FROM CLIENT
            //printf("time to open!!\n");
            //bzero(message,sizeof(message));
            //read(connfd,message,sizeof(message)); //WAIT FOR BOX NAME
            char boxName[1024]; strcpy(boxName,message+6);
            //printf("Open box %s\n", boxName);

            int status = openBox(boxName);
            bzero(message,sizeof(message));
            if(status == 0){
            	//error output:NEXST
    				eventOutput(ip,"ER:NEXST");
                strcpy(message,"ER:NEXST");
            }else if(status == -1){
            	 //error output: OPEND
        			eventOutput(ip,"ER:OPEND");
                strcpy(message,"ER:OPEND");
            }
            else{

                strcpy(message,"OK!");
                //event output: OPNBX
        			 eventOutput(ip,"OPNBX");
            }
            //printf("OPNBX %s\n", boxName);
            //printf("%s\n", message);

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
            //printf("time to create!!\n");
            //bzero(message,sizeof(message));
            //read(connfd,message,sizeof(message)); //WAIT FOR BOX NAME
            char boxName[1024]; strcpy(boxName,message+6);
            //printf("Create box %s\n", boxName);
            //CREATE BOX AND RETURN STATUS

	    		int status = createBox(boxName);
            bzero(message,sizeof(message));
            if (status == -1 || status == -2) {
                //error output: WHAT - should be between 5 and 25 characters and start with alphabetical char
                eventOutput(ip, "ER:WHAT");
                strcpy(message,"ER:WHAT");
            }
            else if(status == 0){
            	//event output: EXIST
					eventOutput(ip,"ER:EXIST");
                strcpy(message,"ER:EXIST");
            }else{
                strcpy(message,"OK!");
					//event output: CREAT
					eventOutput(ip,"CREAT");
            }
            //printf("CREAT %s\n", boxName);
            //printf("%s\n", message);
            write(connfd, message,sizeof(message));

            continue;
		}
		if (strncmp(message, clientCommands[6], 6) == 0){ //DELETEBX
            //printf("time to DELETE!!\n");
            //bzero(message,sizeof(message));
            //read(connfd,message,sizeof(message)); //WAIT FOR BOX NAME
            char boxName[1024]; strcpy(boxName,message+6);
            //printf("Delete box %s\n", boxName);

            int status = deleteBox(boxName);
            bzero(message,sizeof(message));
            if(status == 0){
            	//event output: NEXST
					eventOutput(ip,"ER:NEXST");
                strcpy(message,"ER:NEXST");
            }else if(status == -1){
            	//event output: OPEND
					eventOutput(ip,"ER:OPEND");
                strcpy(message,"ER:OPEND");
            }else if(status == -2){
            	//event output: NOTMT
					eventOutput(ip,"ER:NOTMT");
                strcpy(message,"ER:NOTMT");
            }else{
                strcpy(message,"OK!");
					//event output: DELBX
					eventOutput(ip,"DELBX");
            }
            //printf("DELBX %s\n", boxName);
            //printf("%s\n", message);
            write(connfd, message,sizeof(message));

            continue;

		}
		if (strncmp(message, clientCommands[5], 6) == 0){ //PUTMG
            char msg[1024]; strcpy(msg,message+6);
            bzero(message,sizeof(message));
            //event output: NOOPN
				eventOutput(ip,"ER:NOOPN");
            strcpy(message,"ER:NOOPN");
            //printf("PUTMG!%s\n", msg);
            //printf("%s\n", message);
            write(connfd, message,sizeof(message));
            continue;
		}
		if (strncmp(message, clientCommands[4], 5) == 0){ //NXTMG
            //char boxName[1024]; strcpy(boxName,message+6);
            bzero(message,sizeof(message));
            //event output: NOOPN
				eventOutput(ip,"ER:NOOPN");
            strcpy(message,"ER:NOOPN");
            //printf("NXTMG\n");

            //printf("%s\n", message);
            write(connfd, message,sizeof(message));
            continue;
		}
		if (strncmp(message, clientCommands[7], 6) == 0){ //CLSBX
            char boxName[1024]; strcpy(boxName,message+6);
            bzero(message,sizeof(message));
            //event output: NOOPN
				eventOutput(ip,"ER:NOOPN");
            strcpy(message,"ER:NOOPN");
            //printf("CLSBX %s\n", boxName);
            //printf("%s\n", message);
            write(connfd, message,sizeof(message));
            continue;
		}
		else{
			//if none of the above, then ER:WHAT?
			//event output: WHAT?
			eventOutput(ip,"ER:WHAT?");
			continue;
		}

	}

}

int main(int argc, char* argv[]) {

    int sockfd, connfd;
    struct sockaddr_in servaddr, clientaddr;

    int port_num = atoi(argv[1]);
    if(port_num < 4000){
        printf("Please use a port number greater than 4000\n");
        return 0;
    }

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
	char ip[20];
	bzero(ip,sizeof(ip));
	read(connfd,ip,sizeof(ip));
	//should be ip address
	printf("client's ip address: %s\n", ip);
	char* connected = "connected";
	//event output: connected
	eventOutput(ip,connected);
	//printf("here\n");
	      //threading
	      struct tArgs* arg = malloc(sizeof(struct tArgs));
	      arg->connfd = connfd;
	      arg->tid = tid;
	      strcpy(arg->ip, ip);

	   if (pthread_create(&tid, NULL, interpretCommands,(void*) arg) < 0){
	   	printf("could not thread :( \n");
		return 0;
	   }
 	   //printf("handler assigned\n");
 	   pthread_detach(tid);
 	   //ADD TO THE LL
 	   //printf("adding thread to LL...\n");
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

    }

    if (connfd < 0)
    {
        printf("accept failed");
        return 1;
    }



  close(sockfd);

}

