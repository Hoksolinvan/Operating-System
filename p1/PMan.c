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

Node* head = NULL;




void func_BG(char **cmd){
 
      
	
	//Create another process
	pid_t pid=fork();

	//if the pid is less than 0, this tells us that the fork() for the new process failed
	if(pid<0){
		printf("Fork failed: %s\n",strerror(errno));
		return;
	}
	//We would currently be in the child process
	else if(pid==0){
		

		//if condition to ensure that no fake executable or non-existent executable files is being executed in the command prompt
		if(execvp(cmd[1],&cmd[1])<0){

		//error handler, with message to explain the cause.
		printf("execvp failed: %s\n",strerror(errno));
		//scary exit status number for the failed execvp process
		exit(69);

		}
		
	}
	else if(pid>0)
	{

		usleep(10000); //allows the child process to catch up with the parent process, ensuring that the parent process doesn't execute waitpid before the child process is finished executing
		int status; //error status variable
		pid_t wpid = waitpid(pid,&status,WNOHANG);

		
		//condition to check if the process has been exited, either voluntarily or forcefully.
		if(wpid>=0 && WIFEXITED(status)){

		
		//ensures that the process exitstatus wasn't the scary failed exit status number from the failed execvp
		if(WEXITSTATUS(status)!=69){
		char *dest=(char *)malloc(strlen(cmd[1])+1);
		
		//error handler
		if(dest==NULL){
		printf("Failed to Allocate Memory: %s\n",strerror(errno));
		return;
		}


		//makes a copy and append this string copy to the linkedlist
		strcpy(dest,cmd[1]);
		head=add_newNode(head, pid,dest);
		
	}
				
		}		
	

	}
		
}


void func_BGlist(char **cmd){
	
	//function from modifiedlinkedlist, implementation details are in modifiedlinkedlist.c
	printList(head);
	
}


//function handler for segmentation fault issues
void segmentationHandler(int sig){

	printf("Segmentation Fault Occured! Ending the Program!\n");
	exit(1);
	
}


void func_BGkill(char * str_pid){



	   //handling segmentation fault issues
        signal(SIGSEGV,segmentationHandler);


	  //checking to see if the specified PID exists
        if(PifExist(head,(pid_t)atoi(str_pid))==0){
                printf("Could not find PID\n");
                return;
        }

	//Converts string to pid_t
	pid_t pid=(pid_t)atoi(str_pid);

	//kills the process
	int return_val=kill(pid,9);

	//error handler for failed kill()
	if(return_val==-1){
		printf("Failed to kill process: %s\n",strerror(errno));
		return;
	}

	printf("***KILLED PID: %s***\n",str_pid);

	//clean up the linkedlist
	head=deleteNode(head,pid);

	

}


void func_BGstop(char * str_pid){


	        //handling segmentation fault issues
        signal(SIGSEGV,segmentationHandler);


	 //checking to see if the specified PID exists
        if(PifExist(head,(pid_t)atoi(str_pid))==0){
                printf("Could not find PID\n");
                return;
        }


	//Converts string to pid_t
	pid_t pid=(pid_t)atoi(str_pid);
	
	//pauses the process
	int return_val=kill(pid,19);
	
	//error handler for failed kill()
	if(return_val==-1){
		printf("Failed to stop process: %s\n",strerror(errno));
	}

	return;

}



void func_BGstart(char * str_pid){
	
	//handling segmentation fault issues
	signal(SIGSEGV,segmentationHandler);

	//checking to see if the specified PID exists
        if(PifExist(head,(pid_t)atoi(str_pid))==0){
                printf("Could not find PID\n");
                return;
        }

	//Converts string to pid_t
	pid_t pid=(pid_t)atoi(str_pid);

	//starts the process after its' been paused
	int return_val=kill(pid,18);

	//if the return_value is -1 then it tells us that there was an error of some sort
	if(return_val==-1){

		printf("Failed to start process: %s\n",strerror(errno));
		
	}
	return;
}


void func_pstat(char * str_pid){

	//Local variable declarations and initializations	
	FILE *procfile;
	char buffer[500];
	char *token;
	int count=1;
	double ticks=sysconf(_SC_CLK_TCK);
	char filepath[500];

	//initializing the file path to the pseudo-file system
	snprintf(filepath,sizeof(filepath),"/proc/%s/stat",str_pid);
	
	//opening the file
	procfile=fopen(filepath,"r");

	//if the file value is NULL this means that there is an issue
	if(procfile==NULL){		
		printf("Error: %s\n",strerror(errno));
		return;
	}
	else{
		//adding newline to improve readibility
		printf("\n");

		//iterating through each line of the file
		while(fgets(buffer,sizeof(buffer),procfile)!=NULL){
			//tokenization
			token=strtok(buffer," \n");

			//loop to iterate through the different process information in the stat file
			while(token!=NULL){
				if(count==2){
					printf("comm: %s\n",token);
				}
				else if(count==3){
					printf("State: %s\n",token);
				}
				else if(count==14){
					printf("Utime: %.3f\n",((double)strtoul(token,NULL,10))/ticks);
				}
				else if(count==15){
					printf("Stime: %.3f\n",((double)strtoul(token,NULL,10))/ticks);
				}
				else if(count==24){
					printf("Rss: %lu\n",(unsigned long)strtoul(token,NULL,10));
				}
			
				//increment and move on to the next token delimited either by " " or "\n"	
				count++;
				token=strtok(NULL," \n");	
			}




		}
	}

	//close the file
	fclose(procfile);

	//local variable declaration and initialization
	char secondfilepath[500];
	snprintf(secondfilepath,sizeof(secondfilepath),"/proc/%s/status",str_pid);
	int secondcount=0;

	//opening the file "status" in the pseudo-file system
	procfile=fopen(secondfilepath,"r");

	//error handling
	if(procfile==NULL){
		printf("Error: %s\n",strerror(errno));
		return;
	}else{
		//iterating through all the lines of the "status" file until the desired record is found
		while(fgets(buffer,sizeof(buffer),procfile)!=NULL){

		
			//checks whether the current line has these texts
			if(strncmp(buffer,"voluntary_ctxt_switches:",24)==0){
				token=strtok(buffer," \t");
				token=strtok(NULL," \t");
				printf("Voluntary ctxt_switches: %s",token);
			}
			else if(strncmp(buffer,"nonvoluntary_ctxt_switches:",27)==0){
				token=strtok(buffer," \t");
				token=strtok(NULL," \t");
				printf("Nonvoluntary ctxt_Switches: %s",token);
			}
			
			secondcount++;
		}
		//included to improve readibility
		printf("\n");

		//closing the file
		fclose(procfile);
	}
	
}


//main function template
int main(){
    char user_input_str[50];
    while (true) {
      printf("Pman: > ");
      fgets(user_input_str, 50, stdin);
     // printf("User input: %s \n", user_input_str);
      char * ptr = strtok(user_input_str, " \n");
      if(ptr == NULL){
        continue;
      }
      char * lst[50];
      int index = 0;
      lst[index] = ptr;
      index++;
      while(ptr != NULL){
        ptr = strtok(NULL, " \n");
        lst[index]=ptr;
        index++;
      }
      if (strcmp("bg",lst[0]) == 0){
        func_BG(lst);
      } else if (strcmp("bglist",lst[0]) == 0) {
        func_BGlist(lst);
      } else if (strcmp("bgkill",lst[0]) == 0) {
        func_BGkill(lst[1]);
      } else if (strcmp("bgstop",lst[0]) == 0) {
        func_BGstop(lst[1]);
      } else if (strcmp("bgstart",lst[0]) == 0) {
        func_BGstart(lst[1]);
      } else if (strcmp("pstat",lst[0]) == 0) {
        func_pstat(lst[1]);
      } else if (strcmp("q",lst[0]) == 0) {
	//kills all the process before exiting from the program
	bgkillAll(head);	      
        printf("Bye Bye \n");


        exit(0);
      } else {
        printf("%s: Command Not Found!\n",user_input_str);
      }
    }

  return 0;
}


