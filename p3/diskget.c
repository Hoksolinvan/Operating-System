#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <stdint.h>
#include "FAT_operation.h"


//global variable
char fileName[9]="";
char extension[4]="";

//function prototypes
int search_procedure(char* file_name);
void toUpperString(char *str);
int diskimagechecker(char *str);
int rootcrawler(char* file_memory, char* file_name, char* extension);
int get_last_dot_index(const char *str);
void filecopier(unsigned char* file_memory, unsigned char * file_memory2, int cluster_start, char* subname);
void directory_traverser(unsigned char* file_memory,int cluster_start, int is_root,char* subname);



//boot sector information
struct boot_sector{

	uint16_t bytes_per_sector; //TOTAL SIZE OF DISK
	
}boot_info;


int main(int argc, char * argv[]){


	//input-validation checks

	if(argc<3){

		printf("Lacking the sufficient number of required arguments\n");
		return 1;

	}
	int input_validation_1=diskimagechecker(argv[1]);

	if(argc>3){
		printf("There are too many arguments!\n");
		return 1;
	
	}

	
	//setting up and loading the disk image into main memory for easy manipulation of access (read/write)

	int file_descriptor = open(argv[1],O_RDWR);
	//printf("fd: %s", argv[1]);
	if(file_descriptor==-1){

		perror("Failed to open file\n");
		return 1;
	}


	struct stat file_info;
	if(fstat(file_descriptor,&file_info)==-1){

		perror("Failed to get file information\n");
		close(file_descriptor);
		return 1;

	}

	char* file_memory= mmap(NULL, file_info.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, file_descriptor,0);
	if(file_memory==MAP_FAILED){

		perror("Failed to memory map file\n");
		close(file_descriptor);
		return 1;


	}


	//additional check to verify that the file inserted is a FAT system, eventhough if it doesn't have the .IMA extension

	char system_type[9];
	char* fat_type="FAT12   ";
	strncpy(system_type,file_memory+54,8);
	system_type[9]='\0';

	if((input_validation_1==0 && strcmp(system_type,fat_type)!=0) ||strcmp(system_type,fat_type)!=0 ){

		printf("Please ensure that you know how to use the program \n");
		return 1;

	}

	
	//extract relevant information from the boot sector

	boot_info.bytes_per_sector=file_memory[11]+(file_memory[12]<<8);


	//localize/capitalize the string of the file
	char* input_string=argv[2];
	toUpperString(input_string);
	

	//looking for the file
	int search_ret_val=search_procedure(input_string);

	if(search_ret_val==0){
		printf("That file doesn't exist in the root directory of the disk!\n");
		return 0;
	}

	
	int root_ret_val=0;

	if(search_ret_val==1){

	//function to start going to the file and extracting necessary detail from it
	root_ret_val=rootcrawler(file_memory,fileName,extension);

				if(root_ret_val==0){
					printf("The file doesn't exist\n");
					return 0;
				}
				munmap(file_memory,file_info.st_size);

		close(file_descriptor);

	
	}


	else{
		printf("Invalid File input!\n");
		return 0;
	}

	printf("Thank you, everything worked out in the end! \n");
	return 0;
}






//function to check all possible .ima extensions
int diskimagechecker(char *str){

	char* extension_type= ".ima";
	char* extension_type_upper=".IMA";
	char* extension_type_1=".Ima";
	char* extension_type_2=".IMa";
	char* extension_type_3=".ImA";
	char* extension_type_4=".imA";
	char* extension_type_5=".iMA";
	char* extension_type_6=".iMa";


	char* ret_val_1=strstr(str,extension_type);
	char* ret_val_2=strstr(str,extension_type_upper);
	char* ret_val_3=strstr(str,extension_type);
	char* ret_val_4=strstr(str,extension_type_1);
	char* ret_val_5=strstr(str,extension_type_2);
	char* ret_val_6=strstr(str,extension_type_3);
	char* ret_val_7=strstr(str,extension_type_4);
	char* ret_val_8=strstr(str,extension_type_5);
	char* ret_val_9=strstr(str,extension_type_6);



	//printf("%s %s %s %s %s %s %s %s ",ret_val_1,ret_val_2,ret_val_3,ret_val_4,ret_val_5,ret_val_6,ret_val_7,ret_val_8);
	
	if(ret_val_1==NULL && ret_val_2==NULL && ret_val_3==NULL && ret_val_4==NULL && ret_val_5==NULL && ret_val_6==NULL && ret_val_7==NULL && ret_val_8==NULL && ret_val_9==NULL){
		return 0;
	}
	
	
	

	return 1;
}


//function to parse through the file string and obtain relevant information
int search_procedure(char* file_name){

	char* space_string=" ";
	int i;
	 int index = get_last_dot_index(file_name); //function to get the last dot

		
	//if index!= -1 it is a file because it could have an extension
	if (index != -1) {
       
	   //condition to check whether it is beyond the allowable string length
		if(strlen(file_name)>12){
		
			return 0;
		}
	

	//loop to insert the parsed file_name into the global variable fileName
	for(i=0;i<index;i++){

	
		fileName[i]=file_name[i];

		if(i==8){
			break;
		}
		
	}

	
	//padding for extra spaces
	if(i<8){
		for(i;i<8;i++){
			fileName[i]=space_string[0];
		}
	}

	//loop to insert the parsed file_name into the global variable extension to obtain the extension type
	for(i=0;i<3 && (index + i +1) < strlen(file_name);i++){
		extension[i]=file_name[index+i+1];
		if(i==3){
			break;
		}
	}

		//padding for extra spaces

	 while (i < 3) {
            extension[i++] = space_string[0];
        }
        


	return 1;
	}
	else{
		//otherwise if it doesn't have a '.' in its file name then it is treated as either a file or a directory

		//check to see if it is beyond the length of the allowable file name in FAT12
		if(strlen(file_name)>8){
			
			return 0;
		}


			//loop to insert the parsed file_name into the global variable fileName

		for(i=0;i<strlen(file_name);i++){

		
		fileName[i]=file_name[i];

		if(i==8){
			break;
		}

		

	}

		//padding for extra spaces

	if(i<8){
			for(i;i<8;i++){
			fileName[i]=space_string[0];
		}
	}

		//padding for extra spaces

	for(i=0;i<3;i++){
		extension[i]=space_string[0];
	}


	
	return 1;
	}
	

	

 return 0;
}

//function to obtain first logical cluster of the file to be retrieved from disk
int rootcrawler(char* file_memory, char* file_name, char* extension_type){
	// Duplicate the input strings to ensure local copies are used
	file_name=strdup(fileName);
	extension_type=strdup(extension);

	// Define an empty string for comparison
	const char* empty_string="";

	
//points to the first cluster of the root dir
unsigned char* temporary_pointer = (unsigned char*)(file_memory+(boot_info.bytes_per_sector * 19));


//loops until it reaches an empty directory entry
while(temporary_pointer[0]!= 0x00){
	

	//retrieve the name and extension of the file
	char read_file[9];
	char read_attribute[4];

	strncpy(read_file,temporary_pointer,8);
	read_file[8]='\0';

	strncpy(read_attribute,temporary_pointer+8,3);
	read_attribute[3]='\0';




		//compare to see if the specified file is found
		if(strcasecmp(read_file,file_name)==0 && strcasecmp(read_attribute,extension_type)==0){

			

			uint16_t first_cluster = (uint16_t)(unsigned char)(temporary_pointer[26]) | ((uint16_t)(unsigned char)(temporary_pointer[27] << 8));
			char* rudimentary_subname="";

			//function to actually retrieve the queried file
			filecopier(temporary_pointer,file_memory,first_cluster,rudimentary_subname);

			return first_cluster;
		
		}


temporary_pointer+=32;
}
		
		

	return 0;
}

//helper function to capitalize strings
void toUpperString(char *str) {
    for (int i = 0; i < strlen(str); i++) {
        str[i] = toupper(str[i]);
    }
}




//helper function to acquire the last dot of a string
int get_last_dot_index(const char *str) {
    char *last_dot = strrchr(str, '.');

    if (last_dot == NULL) {
        return -1; // No dot found
    }
	int returnvalue=(int) (last_dot - str);
    return returnvalue;
}


//main function to retrieve file from disk
void filecopier(unsigned char* file_memory, unsigned char * file_memory2, int cluster_start, char* subname){
//file_memory corresponds to the location of the file itself and file_memory2 corresponds to the pointer that is pointing at the boot sector (enables double pointer manipulation)

	//check condition
	if (!file_memory || !file_memory2) {
        fprintf(stderr, "Invalid file memory pointers\n");
        return;
    }

	//sets up, retrieves file name, file extensions, capitalize and concatenate the string to create the filename for the file to be outputted
	char* temporary_pointer = file_memory;
	char read_file[9];
	char read_attribute[4];
	uint32_t file_size;
	char concatenated_string[15]="";
	char* empty_space=" ";
	char* dot=".";

uint16_t cluster_number = (temporary_pointer[26] | (temporary_pointer[27] << 8));

	

	strncpy(read_file,(char*)(temporary_pointer),8);
	read_file[8]='\0';

	strncpy(read_attribute,(char*)(temporary_pointer+8),3);
	read_attribute[3]='\0';

	int k=0;
	
	for(int i=0;i<8 && read_file[i]!=' ';i++){

		concatenated_string[i]=read_file[i];
		k++;

	}
	

	strcat(concatenated_string,dot);
		k++;

	for(int i=0;i<3 && read_attribute[i]!=' ';i++){
		 if (k < sizeof(concatenated_string) - 1) {
            concatenated_string[k]=read_attribute[i];
            k++;
        } else {
            fprintf(stderr, "Concatenated string buffer overflow\n");
            return;
        }
	}

	//retrieves the file size of the file to be retrieved
	memcpy(&file_size,(temporary_pointer+28),sizeof(uint32_t));


	
	//creates a file in the current linux directory for writing into and create a main memory mapping 
	int dest_file = open(concatenated_string, O_RDWR  | O_CREAT, 0666);
        if (dest_file==-1) {
            perror("Error opening destination file");
            return;
        }

	
		//additional set up for promoting versatility
		int return_val_lseek = lseek(dest_file, file_size - 1, SEEK_SET);
    	if (return_val_lseek == -1) {
        perror("Error seeking to end of file");
        close(dest_file);
        return;
    	}

    	int return_val_write = write(dest_file, "", 1);
    	if (return_val_write != 1) {
        perror("Error writing last byte");
        close(dest_file);
        return;
    	}


		struct stat destination;
		if(fstat(dest_file,&destination)==-1){
		perror("Failed to get file information\n");
		close(dest_file);
		return;
		}

	unsigned char* destination_file = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, dest_file, 0);
		if (destination_file == MAP_FAILED) {
			printf("Error: failed to map file memory\n");
			close(dest_file);
			exit(1);
		}



	//necessary variables to iterate through the file, lookup physical sector location, and keep track of the number of bytes left needed to be copied
	int current_cluster = cluster_start;
	int current_cluster2=current_cluster;
    uint32_t bytes_remaining = file_size;
	int physical_address = boot_info.bytes_per_sector * (31+current_cluster2);


	do{
		current_cluster2=(bytes_remaining==file_size) ? current_cluster : getFatEntry(current_cluster2,file_memory2);
		physical_address = boot_info.bytes_per_sector * (31 + current_cluster2);

		int i;
		for(i=0; i<boot_info.bytes_per_sector;i++){
			if(bytes_remaining==0){
				break;
			}
	
		 destination_file[file_size-bytes_remaining]= file_memory2[i+physical_address];
		bytes_remaining--;
		}

	}
	while(bytes_remaining>0);


	


		//cleanup
        if (munmap(destination_file, file_size) == -1) {
            perror("Error unmapping destination file");
			exit(1);

        }
		close(dest_file);
	}







