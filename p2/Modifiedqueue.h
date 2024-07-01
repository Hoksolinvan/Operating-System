#ifndef Modifiedqueue
#define Modifiedqueue
//Include Guard declaration




//struct declarations
typedef struct Node Node;
typedef struct Customers customers;


//Customers structure
struct Customers{

	int customer_id;
	int class_level; //0 for economy, 1 for business
	int arrival_time;//in 1/10th of second
	int service_time;//in 1/10th of second
	double customer_enqueue_time; //time for when the customer enqueued

};


//Node structure
struct Node{
	customers data;
	Node *next;
};



//Function declarations 

//Economy queue
Node *enqueue(Node *head,customers customer);
customers dequeue(Node **head);
int countNodes(Node *head);
void printNodes(Node *head);
#endif
