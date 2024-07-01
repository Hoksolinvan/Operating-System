
all: PMan foo1 foo2 inf args

PMan: PMan.o modifiedlinkedlist.o
	gcc PMan.o modifiedlinkedlist.o -o PMan


PMan.o: PMan.c
	gcc -c PMan.c


modifiedlinkedlist.o: modifiedlinkedlist.c
	gcc modifiedlinkedlist.c -c 

foo1: foo1.o
	gcc foo1.o -o foo1

foo1.o: foo1.c
	gcc -c foo1.c

foo2: foo2.o
	gcc foo2.o -o foo2

foo2.o: foo2.c
	gcc -c foo2.c

inf: inf.o
	gcc inf.o -o inf

inf.o: inf.c
	gcc -c inf.c

args: args.o
	gcc args.o -o args

args.o: args.c
	gcc -c args.c

clean:
	rm -f *.o





