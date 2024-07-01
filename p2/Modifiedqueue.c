#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include "Modifiedqueue.h"

//implementing c queue using linkedlist


Node *enqueue(Node* head,customers customer){

	if(head==NULL){
	
		head=(Node *)malloc(sizeof(Node));

		if(head==NULL){
		perror("Failed to allocate memory!\n");
		return head;
		}
		head->data.customer_id=customer.customer_id;
		head->data.class_level=customer.class_level;
		head->data.arrival_time=customer.arrival_time;
		head->data.service_time=customer.service_time;
		head->next=NULL;

	}
	else{

		Node *current=head;
		while(current->next!=NULL){

			current=current->next;
		}

		current->next=(Node *)malloc(sizeof(Node));
		if(current->next==NULL){
			perror("Failed to allocate memory!\n");
			return current->next;
		}
		current->next->data.customer_id=customer.customer_id;
		current->next->data.class_level=customer.class_level;
		current->next->data.arrival_time=customer.arrival_time;
		current->next->data.service_time=customer.service_time;
		current->next->next=NULL;


	}
	return head;
}


customers dequeue(Node **head){
	if(*head==NULL){
		
		perror("There is nothing to dequeue as the queue is empty!\n");
		return (*head)->data;

	}
		
		Node *current=*head;
		customers dequeued_data = current->data;
		(*head)=(*head)->next;
		free(current);
		return dequeued_data;

}


int countNodes(Node *head){
	int count=0;
	if(head==NULL){
		return count;
	}
	else{
	Node *current=head;
		
		while(current!=NULL){

		count++;
		current=current->next;
		}

		return count;

	}




}


void printNodes(Node *head){


	Node *current=head;

	while(current!=NULL){

		int current_customerid=current->data.customer_id;

		printf("Customer ID currently in the queue is: %d",current_customerid);

		current=current->next;
	}





}

 

