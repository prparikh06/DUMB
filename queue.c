#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//using linked lsit to create a queue
int size  = 0;
struct Node{
	char* data;
	struct Node* next;
};

//logic: since queue is FIFO, we can add the newest node to the end of the list, and remove the front

//enqueue: add to end of LL
void enqueue(struct Node** front, char* message){
	struct Node* new = (struct Node*) malloc(sizeof(struct Node));
	new->data = malloc(strlen(message)+1);
	new->next = NULL;
	strcpy(new->data, message);

	if (size == 0){ //no nodes added
		*front = new;
		size++;
		return;
	}
	struct Node* ptr = *front;
	while (ptr->next != NULL) ptr = ptr->next;
	ptr->next = new;
	size++;
	return;
}
//dequeue: remove from front of LL
struct Node* dequeue(struct Node** front){
	if (size == 0) //nothing to dequeue
		return NULL;
	struct Node* ptr = *front;
	*front = ptr->next;
	//free(ptr);
	size--;
	return ptr;
}

int getSize(){
	return size;
}

void printList(struct Node* front){

	for(struct Node* ptr = front; ptr != NULL; ptr = ptr->next) printf("%s\t", ptr->data);
	printf("\n");

}
/*
int main(){
	//queue is front node of LL
	struct Node* queue = (struct Node*) malloc(sizeof(struct Node));
	enqueue(&queue, "parikh");
	enqueue(&queue, "priya");
	dequeue(&queue);

	printList(queue);

}
*/
