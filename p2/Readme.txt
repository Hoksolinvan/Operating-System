
The folder should contain 4 essential files required for running the program:
- ACS.c (The main code base that would include all the necessary functions for this assignment)
- Modifiedqueue.c (The source code which includes the implementation details for the linkedlist-based queue that will be used for this assignment to represent the different economy and business queues)
- Modifiedqueue.h (The header file that would contain the necessary function prototypes for the Modifiedqueue) 
- Makefile (The makefile for compiling and linking the necessary files together to create the main program, "ACS")


To run the program it is essential that there be ACS.c, Modifiedqueue.c and Modifiedqueue.h.

Typing the command, "make" will cause the creation of the necessary object files that are needed for the program. Therefore, the ACS.o and Modifiedqueue.o would be essential object files for the program
and when they are linked together will cause create the main ACS executable program.

To run the program, type ./ACS [argument1] onto the command line interface. Where "argument1" should be the customer text files that abides by the rules of the convention of the assignment. The program
will utilize necessary error-handling procedures to ensure that mistakes are avoided.

The input file here is called, "customers1.txt"
