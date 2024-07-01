all: ACS 


ACS: ACS.o Modifiedqueue.o
	gcc -Wall -pthread ACS.o Modifiedqueue.o -o ACS

ACS.o: ACS.c
	gcc -c ACS.c

Modifiedqueue.o: Modifiedqueue.c
	gcc -c Modifiedqueue.c



clean:
	rm -f *.o
