#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

//function get acquire fat entry
int getFatEntry(unsigned int x, char *file_memory){

	unsigned int result;
	unsigned int XL; //low-byte
	unsigned int XH; //high-byte


	if((x%2)==0){

		XL= file_memory[512+((3*x)/2)] & 0xFF;
		XH= file_memory[512+(1+((3*x)/2))] & 0x0F;
		result=(XH<<8)+XL;


	}
	else{

		XH= file_memory[512+((3*x)/2)] & 0xF0;
		XL= file_memory[512+(1+((3*x)/2))] & 0xFF;
		result=(XH>>4)+(XL<<4);


	}

	return result;

}


