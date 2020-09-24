#include <stdio.h>
#include <stdlib.h>
#include "tokenizer.h"

/* Implementation of Linked List in C*/

void free_list(TokenNode *front)
{
	// Like for other linked list functions, check if the front is null or not
	if (front == NULL){
		return;
	}

	//Set a pointer so we can iterate nicely
	TokenNode *ptr = front;
	while (ptr->next != NULL) 
	{
		// This would be like any traversal but we are freeing each memory of the pointers
		TokenNode *tmp = ptr->next;
		free(ptr); // like here
		ptr = tmp;
	}

	// then we free the rest
	free(ptr);
}

// Using the front as a input and given a data input that you want to store it will make a newNode
// and store that to the front of the list
TokenNode *addNode(TokenNode *front, char *data)
{
	TokenNode *newNode = malloc(sizeof(TokenNode)); //malloc space for the newNode that you are creating
	newNode->data = data;
	newNode->next = NULL;

	// if front is null, which is most likely when your first adding to front then
	// make newNode the front
	if (front == NULL)
	{
		front = newNode;
		//free(newNode);
		return front;
	}

	// Otherwise front is not null and you can keep adding to the front node
	(*newNode).next = front->next;
	front->next = newNode;

	// In the end return front
	return front;
}

TokenNode* deleteNode(TokenNode *front, char *data)
{
	// Check if the front of the list is actually there
	if (front == NULL){
		return NULL; // Don't return anything significant if its not there
	}

	/* Now we can set two pointers a ptr to the front and a pointer to the previous
	element. Which we will use to iterate through the list */
	TokenNode *ptr = front;
	TokenNode *prev = NULL;

	// Now we iterate through the linked list so that we can delete the elements using the condition
	// ptr != NULL which we will traverse later using ptr = ptr->next
	while (ptr != NULL)
	{
		if ((*ptr).data == data)
		{
			// We reached the data we were looking for
			// We do it in the loop because there might be more than one element we are looking for
			break;
		}
		
		//Change the pointer locations
		prev = ptr;
		ptr = (*ptr).next;
	}
	
	// after the loop is done, we checked all the conditions so we can exit and return the front of the list 
	// which should be modified.
	if(ptr != NULL){
		if(prev != NULL){
			prev->next = ptr->next;
		} else {
			front = ptr->next;
		}
		free(ptr);
	}
	return front;
}

// Meant for testing purposes, to see if all the nodes print out or if there is a memory leak somewhere
void traverseList(TokenNode *front)
{
	TokenNode *ptr = front;
	while (ptr != NULL)
	{
		printf("%s \t", (*ptr).data); // Print out the corresponding data
		ptr = (*ptr).next;
	}
}


/* int main(int argc, char **argv)
{
	Node *front = NULL;
	front = addNode(front, argv[1]);
	front = addNode(front, "what");
	front = addNode(front, "wd");
	front = addNode(front, "okay");
	front = addNode(front, "okay");
	front = deleteNode(front, "okay");
	front = deleteNode(front, "okay");
	traverseList(front);
	free_list(front);
	return 0;
} */
