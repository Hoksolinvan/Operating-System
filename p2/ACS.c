#include <stdio.h>
#include <string.h>
#include "Modifiedqueue.h"
#include <errno.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

//global variables
double total_waiting;
double total_business_waiting;
double total_economy_waiting;
int business_num;
int economy_num;
int total_num_customers;

//function prototype(s)

int Mutex_Convar_Creator();
int Mutex_Convar_Destroyer();

//start_time struct to save start_time
static struct timeval start_time;


//time measuring function
double getCurrentSimulationTime(){

	struct timeval cur_time;
	double cur_secs, init_secs;

	init_secs = (start_time.tv_sec + (double) start_time.tv_usec / 1000000);

	gettimeofday(&cur_time,NULL);
	cur_secs = (cur_time.tv_sec + (double) cur_time.tv_usec / 1000000);
	
	return cur_secs - init_secs;



}


//Queues

Node *Economy=NULL;
Node *Business=NULL;



//pthread functions

void *customer_thread(void *customer_details);
void *employee_thread(void *customer_details);


//mutex locks

pthread_mutex_t economy_mutex;
pthread_mutex_t business_mutex;
pthread_mutex_t primary_wait_mutex;
pthread_mutex_t secondary_wait_mutex;

//Conditional Variables

pthread_cond_t ACS_cond;


//Customer thread function
void *customer_thread(void *customer_details){

//extracts the struct value from customer
customers *clients=(customers *)customer_details;
int sleeptime=clients->arrival_time*100000;

//simulates customer arrival time and insert customer's enqueue time into the simulation
usleep(sleeptime);
clients->customer_enqueue_time=getCurrentSimulationTime();
printf("A Customer arrives: %d\n",clients->customer_id);


//checks whether customers are in economy class or business class
if( clients->class_level==0){


//lock critical section for economy queue
pthread_mutex_lock(&economy_mutex);
	printf("Customer %d has entered the Economy queue(%d), and the length of the queue is %d\n",clients->customer_id,clients->class_level,countNodes(Economy));
	
	//increment the number of economy customers and enqueues them
	economy_num++;
	Economy=enqueue(Economy,*clients);

	//signals to any currently waiting clerks
	pthread_cond_signal(&ACS_cond);


pthread_mutex_unlock(&economy_mutex);


}
else if(clients->class_level==1){
	
//lock critical section for business queue	
	pthread_mutex_lock(&business_mutex);
	printf("Customer %d has entered the Business queue(%d), and the length of the queue is %d\n",clients->customer_id,clients->class_level,countNodes(Business));
	
	//increment the number of business customers and enqueues them
	business_num++;
	Business=enqueue(Business,*clients);
	
	//signals to any currently waiting clerks
	pthread_cond_signal(&ACS_cond);

	pthread_mutex_unlock(&business_mutex);

}
return NULL;
}


//clerk thread function
void *employee_thread(void *employee_details){

//obtains clerk unique ID
int employee_id=*((int *)employee_details);


//main while loop to ensure that there are still customers left to be serviced
while(total_num_customers>0){

	//if there aren't any customers waiting in a queue at this moment the clerk waits until one of the customer signals them
	pthread_mutex_lock(&primary_wait_mutex);	
	while(countNodes(Business)==0 && countNodes(Economy)==0 && total_num_customers>0){
		
		pthread_cond_wait(&ACS_cond,&primary_wait_mutex);
		
	}
	pthread_mutex_unlock(&primary_wait_mutex);
	

	//checks the business queue because it has more priority and then checks the economy queue afterwards if the condition was not initally satisfied
	if(countNodes(Business)>0){

		//local variable declaration
		customers current_customer;
		int customer_servicetime=0;
		
		//critical section code for decreasing the total number of customers left and dequeuing customers currently waiting in line
		pthread_mutex_lock(&business_mutex);
	
			
		Node **Business_pointer=&Business;	
		current_customer=dequeue(Business_pointer);
		

		total_num_customers--;

		
		
		pthread_mutex_unlock(&business_mutex);

		customer_servicetime=current_customer.service_time*100000;

		//local variable to keep track of the time that the customer had to wait in line before being serviced
		double processed_arrival_time = (double)(current_customer.arrival_time)/10;
		double time_waited=getCurrentSimulationTime()-processed_arrival_time;

		printf("Clerk %d has started taking care of customer %d\n",employee_id,current_customer.customer_id);

		printf("Customer %d (Business) spent %.2f seconds waiting before being served!\n",current_customer.customer_id,time_waited);	
				
		//simulates service time and calculates the time required to service the customers	
		usleep(customer_servicetime);
		double processing_time=getCurrentSimulationTime()-processed_arrival_time-time_waited;
		
		//prints aftermath
		printf("After %.2f seconds, clerk %d has finished taking care of customer %d\n",processing_time,employee_id,current_customer.customer_id);


		//critical section code for incrementing the necessary amount for wait time
		pthread_mutex_lock(&secondary_wait_mutex);
		total_waiting+=time_waited;
		total_business_waiting+=time_waited;
		
		pthread_mutex_unlock(&secondary_wait_mutex);


		continue;		
	}
	else if(countNodes(Economy)>0){

		//local variable declaration
		customers current_customer;
		int customer_servicetime=0;


		//critical section code for decreasing the total number of customers left and dequeuing customers currently waiting in line for economy
		pthread_mutex_lock(&economy_mutex);
		
		Node **Economy_pointer=&Economy;
		current_customer=dequeue(Economy_pointer);
		
		total_num_customers--;
		
		pthread_mutex_unlock(&economy_mutex);

		customer_servicetime=current_customer.service_time * 100000;

		//local variable to keep track of the time that the customer had to wait in line before being serviced
		double processed_arrival_time=(double)(current_customer.arrival_time)/10;
		double time_waited=getCurrentSimulationTime()-processed_arrival_time;

		printf("Clerk %d has started taking care of customer %d\n",employee_id,current_customer.customer_id);

		printf("Customer %d (Economy) spent %.2f seconds waiting before being served!\n",current_customer.customer_id,time_waited);
	

		//simulates service time and calculates the time required to service the customers
		usleep(customer_servicetime);
		
		double processing_time=getCurrentSimulationTime()-processed_arrival_time-time_waited;
		
		//prints aftermath
		printf("After %.2f seconds, clerk %d has finished taking care of customer %d\n",processing_time,employee_id,current_customer.customer_id);
		
		//critical section code for incrementing the necessary amount for wait time
		pthread_mutex_lock(&secondary_wait_mutex);
		total_waiting+=time_waited;
		total_economy_waiting+=time_waited;
		
		pthread_mutex_unlock(&secondary_wait_mutex);
		
		continue;
	}




	



}
	//takes all remaining clerk thread off the waiting queue for conditional variables and deallocate memory
	pthread_cond_broadcast(&ACS_cond);

	free(employee_details);

	return NULL;
}





int main(int argc, char* argv[]) {
    // Input validation
    if (argc < 2) {
        printf("No file was inserted!\n");
        return 1;
    } else if (strcmp(argv[1], "") == 0) {
        printf("Please try again!\n");
        return 1;
    } else if (argc > 2) {
        printf("Please ensure that you know how to use the program\n");
        return 1;
    }

    // Variable declaration
    FILE* CustomerFile;
    char line[100];
    char* token;
    const char delimiters[5] = " \n:,";
    int total_customers;

    // File validation handling
    CustomerFile = fopen(argv[1], "r");
    if (CustomerFile == NULL) {
        printf("Failed to open file: %s\n", strerror(errno));
        return 1;
    }

    // Reads the total number of customers from the file
    if (fgets(line, sizeof(line), CustomerFile) != NULL) {
        token = strtok(line, delimiters);
        total_customers = atoi(token);

        if (total_customers == 0) {
            printf("There aren't any customers to serve today, bye bye!\n");
            fclose(CustomerFile); // Close the file if no customers
            return 0;
        }
    } else {
        printf("Failed to read number of customers from file\n");
        fclose(CustomerFile); // Close the file on failure
        return 1;
    }

    // Creates the array that would store the details of customers
    customers customerarray[total_customers];

    // Read customer details from the file into customerarray
    int i = 0;
    while (i < total_customers && fgets(line, sizeof(line), CustomerFile) != NULL) {
        // Text file validation, if the current line is not EOF continue reading from the next line
        if (line[0] == '\n' || line[0] == '\0') {
            continue;
        }

        // Tokenize the line
        token = strtok(line, delimiters);

        // If the token results in these things but not EOF continue reading from the next line
        if (strcmp(token, "") == 0 || token == NULL) {
            continue;
        }

        // Temporary variables
        int customer_id, class_level, arrival_time, service_time;
        int count = 0;

        // Initialization procedures
        while (token != NULL) {
            if (count == 0) {
                customer_id = atoi(token);
                count++;
                customerarray[i].customer_id = customer_id;
            } else if (count == 1) {
                class_level = atoi(token);
                count++;
                customerarray[i].class_level = class_level;
                if (class_level == 1) {
                    business_num++;
                } else if (class_level == 2) {
                    economy_num++;
                }
            } else if (count == 2) {
                arrival_time = atoi(token);

                // If arrival_time is negative, then end the program with an error message
                if (arrival_time < 0) {
                    printf("There exists a negative time entry for arrival_time. Ending the program.\n");
                    return 1;
                }
                count++;
                customerarray[i].arrival_time = arrival_time;

            } else if (count == 3) {
                service_time = atoi(token);

                // If service_time is negative, then end the program with an error message
                if (service_time < 0) {
                    printf("There exists a negative time entry for service_time. Ending the program.\n");
                    return 1;
                }
                count++;
                customerarray[i].service_time = service_time;
            }
            token = strtok(NULL, delimiters);
        }
        i++;
    }

    fclose(CustomerFile); // Close the file after reading all customers

    // Initialization of global variable total_num_customers
    total_num_customers = total_customers;

    // Thread variable declaration
    pthread_t employees[5];
    pthread_t customers[total_customers];

    // Start the simulation
    double cur_simulation_secs;
    gettimeofday(&start_time, NULL);

    // Function for creation of mutex and condition variables
    int return_val1 = Mutex_Convar_Creator();

    if (return_val1 == 1) {
        printf("There was an error with creating the mutexes and condition variables\n");
        return 1;
    }

    // Clerk thread creation
    for (int i = 0; i < 5; i++) {
        int* employee_id = (int*)malloc(sizeof(int));
        if (employee_id == NULL) {
            printf("Failed to allocate memory\n");
            return 1;
        }

        *employee_id = i;

        int result = pthread_create(&employees[i], NULL, employee_thread, (void*)employee_id);
        if (result != 0) {
            printf("Error creating employees thread: %d\n", result);
            return i;
        }

        printf("Clerk %d started working!\n", i);
    }

    // Customer thread creation
    for (int i = 0; i < total_customers; i++) {
        int result = pthread_create(&customers[i], NULL, customer_thread, (void*)&customerarray[i]);
        if (result != 0) {
            printf("Error creating customers thread: %d\n", result);
            return 1;
        }
    }

    // Join customer threads
    for (int i = 0; i < total_customers; i++) {
        pthread_join(customers[i], NULL);
    }

    // Join employee threads
    for (int i = 0; i < 5; i++) {
        pthread_join(employees[i], NULL);
    }

    // Cleans up and destroy Mutex and conditional variables
    int return_val2 = Mutex_Convar_Destroyer();

    if (return_val2 == 1) {
        printf("There was an error with destroying the mutexes and condition variables\n");
        return 1;
    }

    // Final statistic output
    printf("----------------------------------------------------------\n");
    printf("Over the course of a %.0f second simulation:\n", getCurrentSimulationTime());
    printf("----------------------------------------------------------\n");
    printf("We served %d customers, of which %d were business and %d were economy!\n", business_num + economy_num, business_num, economy_num);

    printf("Customers spent a total of %.0f seconds waiting!\n", total_waiting);

    printf("The average waiting time for all customers is: %.2f seconds.\n\n", total_waiting / (business_num + economy_num));

    printf("Business-class customers spent a total of %.0f seconds waiting!\n", total_business_waiting);

    // Condition to check whether there were any business customers throughout the entire session
    if (business_num > 0) {
        printf("The average waiting time for all business-class customers is: %.2f seconds.\n\n", total_business_waiting / business_num);
    } else {
        printf("The average waiting time for all business-class customers is: 0.00 seconds.\n\n");
    }

    printf("Economy-class customers spent a total of %.0f seconds waiting!\n", total_economy_waiting);

    // Condition to check whether there were any economy customers throughout the entire session
    if (economy_num > 0) {
        printf("The average waiting time for all economy-class customers is: %.2f seconds.\n\n", total_economy_waiting / economy_num);
    } else {
        printf("The average waiting time for all economy-class customers is: 0.00 seconds.\n\n");
    }

    return 0;
}

int Mutex_Convar_Creator(){

//Mutex & Conditional variable initialization

	if(pthread_mutex_init(&primary_wait_mutex,NULL)!=0){

		printf("Primary wait Mutex Initialization failed!\n");
		return 1;

	}

	if(pthread_mutex_init(&secondary_wait_mutex,NULL)!=0){

		printf("Secondary wait Mutex Initialization failed!\n");
		return 1;

	}

	if(pthread_mutex_init(&economy_mutex,NULL)!=0){

		printf("Economy Mutex initialization failed!\n");
		return 1;


	}




	if(pthread_mutex_init(&business_mutex,NULL)!=0){

		printf("Business Mutex initialization failed!\n");
		return 1;


	}



	if(pthread_cond_init(&ACS_cond,NULL)!=0){

		printf("ACS condition variables initialization failed! \n");
		return 1;


}

	return 0;

}


int Mutex_Convar_Destroyer(){


    // Destroy mutexes and condition variables
   if( pthread_mutex_destroy(&primary_wait_mutex)!=0){

	printf("Failed to destroy primary wait mutex\n");
	return 1;	
	   
   }
    if(pthread_mutex_destroy(&secondary_wait_mutex)!=0){

	printf("Failed to destroy secondary wait mutex\n");
   	return 1;
   
    }

    if(pthread_mutex_destroy(&economy_mutex)!=0){

	    printf("Failed to destroy economy mutex\n");
	    return 1;

	}

    
    if(pthread_mutex_destroy(&business_mutex)!=0){

	    printf("Failed to destroy business mutex\n");

	    return 1;

	}

    
    if(pthread_cond_destroy(&ACS_cond)!=0){

	    printf("Failed to destroy ACS condition variable\n");
	    return 1;

    }

    return 0;

}

