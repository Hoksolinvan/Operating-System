#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include "ModifiedLinkedList.h"

 
Node * add_newNode(Node* head, pid_t new_pid, char * new_path){


	//if the head struct was initially NULL
	    if(head==NULL)
        {

                head=(Node *)malloc(sizeof(Node));

		//if trying to allocate memory for head and it has the value of NULL still
                if(head==NULL){
                perror("Failed to allocate memory");
                exit(1);
                }

                head->pid=new_pid;
                head->path=new_path;
                return head;
        }
        else{

        //creating a pointer to iterate through the linkedlist
        Node *current=NULL;
        current=head;

        //iterating through the linked list
        while(current->next!=NULL){
        current=current->next;
        }

        //Creating the node and initializing the values
        Node *new_node= (Node *)malloc(sizeof(Node));

        //Exception Handling for memory management
        if(new_node==NULL){

        perror("Failed to allocate memory");
        exit(1);

        }

        new_node->path=new_path;
        new_node->pid=new_pid;

        //adding the new node to the current path
        current->next=new_node;


        return head;
        }
}


Node * deleteNode(Node* head, pid_t pid){
	// your code here
	//
	
	//no Nodes existed in the first place, therefore nothing can be deleted
	if(head==NULL){

	return NULL;
	}

	
	//Case for pid value being contained at head
	if(head->pid==pid){

	//creating another pointer to the struct pointed by head pointer as well
	Node *temp = head;

	//moving the new head location
	head=head->next;

	//deallocating memory and returning the new head position
	free(temp);
	return head;
	}

	//creating another pointer to iterate through the entire linkedlist
	Node * current=head;


	while(current->next!=NULL){
		

		//handling the event where current->next has the specified pid and deallocating memory for efficient memory use
		if(current->next->pid==pid){
			Node *temp=current->next;
			current->next=current->next->next;
			free(temp);
			
		}

		current=current->next;

	}

	//return head
	return head;
}

void printList(Node *node){
	
	//initial number of background jobs
	int count=0;

	//no background job generated by PMan exists
	if(node==NULL){
	printf("No background jobs.\nTotal background jobs: 0\n");
	}	
	else if(node!=NULL){
	//creating pointer to iterate through the loop
	Node *current=node;
	while(current!=NULL){
		printf("%d: %s\n",(int)current->pid,current->path);
		//iterating through the loop and incrementing count
		current=current->next;		
		count++;
	}

	//printing out total # of background jobs
	printf("Total background jobs: %d\n",count);
	
}
}

int PifExist(Node *node, pid_t pid){
	

	//return value of 0 represents Pdoesn'texist and 1 represents PExists
	//
	
	//if the node was NULL from the beginning this tells us that pid doesn't exist
	if(node==NULL){
		return 0;
	}

	//creating a pointer to iterate throught the node
	Node *current=node;


	//iteration check
	while(current!=NULL){

		if(current->pid==pid){
			return 1;
		}
	}




	//default return value indicates that the specified p ID doesn't exist
  	return 0;
}


//function to kill all the background processes
void bgkillAll(Node *node){

	//if the head node is empty that means that there were never any background processes to begin with
	if(node==NULL){
		return;
	}

	//start by creating a pointer to the head of the linkedlist
	Node *current=node;

	while(current!=NULL){

	//kill the process
        int return_val=kill(current->pid,9);

        //error handler for failed kill()
        if(return_val==-1){
                printf("Failed to stop process: %s\n",strerror(errno));
        }

 	//iterate through the linkedlist	
		current=current->next;

	}


	return;
}
