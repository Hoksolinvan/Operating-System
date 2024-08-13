all: diskinfo disklist diskget diskput


diskinfo: diskinfo.o FAT_operation.o
	gcc diskinfo.o FAT_operation.o -o diskinfo

diskinfo.o: diskinfo.c
	gcc -c diskinfo.c

disklist: disklist.o FAT_operation.o
	gcc disklist.o FAT_operation.o -o disklist

disklist.o: disklist.c
	gcc -c disklist.c

diskget: diskget.o FAT_operation.o
	gcc diskget.o FAT_operation.o -o diskget

diskget.o: diskget.c
	gcc -c diskget.c


diskput: diskput.o FAT_operation.o
	gcc diskput.o FAT_operation.o -o diskput

diskput.o: diskput.c
	gcc -c diskput.c

FAT_operation.o: FAT_operation.c
	gcc -c FAT_operation.c


clean:
	rm -f *.o
