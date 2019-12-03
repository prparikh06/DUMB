#ifndef _queue_h_
#define _queue_h_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//functions for the queue
struct Node{
	char* data;
	struct Node* next;

};

void enqueue(struct Node** front, char* message);
char* dequeue(struct Node** front);
int getSize();
void printList(struct Node* front);

#endif
