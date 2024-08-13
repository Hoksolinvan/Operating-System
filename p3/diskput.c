#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include "FAT_operation.h"

// Global variable for the boot sector information
int file_existence=0;

struct boot_sector {
    uint16_t bytes_per_sector;
    uint16_t sectors_per_FAT;
    uint16_t total_sector_count;
} boot_info;

// Function prototypes
int diskimagechecker(char *str); 
void toUpperString(char *str); 
char* fileparser(char* file_name); 
int get_last_dot_index(const char *str);
int get_last_slash_index(const char *str); 
int Free_disk_size(char *file_memory); 
int getNextEmptyFatIndex(char *file_memory); 
void fileSignaturemaker(char* file_name,int fileSize,int first_cluster, int empty_first_cluster, char* file_memory); 
void dataInserter(char* file_memory, int first_cluster,int file_size,char* inserted_file);
void FatEntryUpdate(int count, char* file_memory, int insert_value);
uint16_t directorytraverser(char* input_string, char* file_name, char* file_memory,int is_root,uint16_t start_cluster);



int main(int argc, char *argv[]) {


    //condition to check the # of arguments
    if (argc < 3) {
        printf("Lacking the sufficient number of required arguments\n");
        return 1;
    }
    int input_validation_1=diskimagechecker(argv[1]); //check if the second of argument is the disk image file

    //trivially dismiss commands with too many arguments
    if (argc > 3) {
        printf("There are too many arguments!\n"); 
        return 1;
    }
    

    //opening and setting up the disk image file, loading into the main memory region
    int file_descriptor = open(argv[1], O_RDWR);
    if (file_descriptor == -1) {
        perror("Failed to open file\n");
        close(file_descriptor);
        return 1;
    }
    
    struct stat file_info;
    if (fstat(file_descriptor, &file_info) == -1) {
        perror("Failed to get file information\n");
        close(file_descriptor);
        return 1;
    }

    
    char *file_memory = mmap(NULL, file_info.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, file_descriptor, 0);
    if (file_memory == MAP_FAILED) {
        perror("Failed to memory map file\n");
        close(file_descriptor);
        return 1;
    }




    //additional edge-case check for FAT12 disk images without .IMA extension
    char system_type[9];
	char* fat_type="FAT12   ";
	strncpy(system_type,file_memory+54,8);
	system_type[9]='\0';

	if((input_validation_1==0 && strcmp(system_type,fat_type)!=0) ||strcmp(system_type,fat_type)!=0 ){

		printf("Please ensure that you know how to use the program \n");
        munmap(file_memory,file_info.st_size);
        close(file_descriptor);
		return 1;

	}
    

    //extracting information from boot sector regarding current disk image
    boot_info.bytes_per_sector = file_memory[11] + (file_memory[12] << 8);
    boot_info.sectors_per_FAT = file_memory[22] + (file_memory[23] << 8);
    boot_info.total_sector_count = file_memory[19] + (file_memory[20] << 8);


    //extract the filename of the path (e.g,. /s1/s2/s3/f1   extract f1 throught the fileparser function)
    char *input_string = argv[2];
    char *read_file_name = fileparser(input_string);
   

    //setting up the file to be written onto disk and loading into main memory for easy write/read
    int real_file = open(read_file_name, O_RDONLY);
    if (real_file == -1) {
        printf("The file that you are trying to insert does not exist, File not Found!\n");
        free(read_file_name);
        munmap(file_memory,file_info.st_size);
        close(real_file);
        return 1;
    }
      

    struct stat insert_file_info;
    if (fstat(real_file, &insert_file_info) == -1) {
        perror("Failed to get file information\n");
        close(file_descriptor);
        return 1;
    }
    

    char *second_file_memory = mmap(NULL, insert_file_info.st_size, PROT_READ , MAP_SHARED, real_file, 0);
    if (second_file_memory == MAP_FAILED) {
        perror("Failed to memory map file\n");
        close(file_descriptor);
        return 1;
    }


    //check to see if there are still available disk spaces
    int free_disk_size_left=Free_disk_size(file_memory);
    int insert_file_size=insert_file_info.st_size;
    if(insert_file_size>free_disk_size_left){
        printf("There aren't enough free space in the disk left!\n");
        return 1;
    }

    
    //capitalize the path, filenames, and creation of duplicate to avoid heavy usage of references
    toUpperString(input_string);
    char* file_name=strdup(input_string);
   file_name=fileparser(file_name);


    //extract the logical start directory of the directory that contains the file
    uint16_t logical_start_of_dir=directorytraverser(input_string,file_name,file_memory,1,19);
    


    //a specific subdirectory of the path could not located
    if(logical_start_of_dir==0){

    close(file_descriptor);
    close(real_file);
    munmap(file_memory, file_info.st_size);
    munmap(second_file_memory,insert_file_info.st_size);
    printf("Could not locate the proper directories, The directory was not found!\n");
    return 1;

    }
    else if(logical_start_of_dir==69){

    close(file_descriptor);
    close(real_file);
    munmap(file_memory, file_info.st_size);
    munmap(second_file_memory,insert_file_info.st_size);
    printf("Another file with the same name already exists in the folder\n");
    return 1;
    }

    //getting the first logical sector of the file
    int first_logical_start_of_file=getNextEmptyFatIndex(file_memory)-31;


    //insertion of directory entry into the directory that it is being contained wihtin 
    fileSignaturemaker(read_file_name,insert_file_size,logical_start_of_dir,first_logical_start_of_file,file_memory);


    //final insertion to insert data of the file itself
    dataInserter(file_memory,first_logical_start_of_file,insert_file_size,second_file_memory);

    // Clean up
    free(file_name);
    free(read_file_name);
    close(file_descriptor);
    close(real_file);
    munmap(file_memory, file_info.st_size);
    munmap(second_file_memory,insert_file_info.st_size);
    return 0;
}







//function to check if file has a .ima extension
int diskimagechecker(char *str) {
    char* extension_type = ".ima";
    char* ret_val = strrchr(str, '.'); // Find the last occurrence of '.'
    return (ret_val != NULL && strcasecmp(ret_val, extension_type) == 0) ? 1 : 0;
}


//function to capitalize a string
void toUpperString(char *str) {
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++) {
        if (str[i] == '.' || str[i] == '/') {
            continue;
        }
        str[i] = toupper(str[i]);
    }
}


//function to extract the last file or dir in a file path
char* fileparser(char* file_name) {
    char *placeholder_string;
    int get_last_slash = get_last_slash_index(file_name);

    if (get_last_slash != -1) {
        size_t length = strlen(file_name) - get_last_slash - 1;
        placeholder_string = malloc(length+1);
        if (placeholder_string == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            return NULL;
        }
        strcpy(placeholder_string, file_name + get_last_slash + 1);
        return placeholder_string;
    }
    return strdup(file_name);
}

//function to get the last dot of index and slashes


int get_last_dot_index(const char *str) {
     if (str == NULL) {
        fprintf(stderr, "Error: NULL input string\n");
        return -1;
    }
    char *last_dot = strrchr(str, '.');
    return (last_dot == NULL) ? -1 : (int)(last_dot - str);
}


int get_last_slash_index(const char *str) {
    char *last_slash = strrchr(str, '/');
    return (last_slash == NULL) ? -1 : (int)(last_slash - str);
}




//function to calculate the number of free space available on the disk
int Free_disk_size(char* file_memory){

	//count variables
	int count=0;
	int fat_entries = ((8*boot_info.sectors_per_FAT*boot_info.bytes_per_sector)/12);
	
	for(int i=2; i<fat_entries;i++){
			
		if((getFatEntry(i,file_memory)==0x000) && ((i+31)<boot_info.total_sector_count)){
			count++;
		}
	
	}

	return boot_info.bytes_per_sector*count;
}




//function to update the fat entry whenever a data is inserted
void FatEntryUpdate(int count, char* file_memory, int insert_value) {

    // Calculate the byte offset for the entry
    int byteOffset = boot_info.bytes_per_sector + (3 * count) / 2;
    char* temporary_pointer = file_memory + byteOffset;

    if ((count % 2) == 0) {
        // Even n
        temporary_pointer[0] = insert_value & 0xFF;
        temporary_pointer[1] = (temporary_pointer[1] & 0xF0) | ((insert_value >> 8) & 0x0F);
    } else {
        // Odd n
        temporary_pointer[0] = (temporary_pointer[0] & 0x0F) | ((insert_value << 4) & 0xF0);
        temporary_pointer[1] = (insert_value >> 4) & 0xFF;
    }


}





//function to find the specific subdirectory or directory that the file is located in
uint16_t directorytraverser(char* input_string, char* file_name, char* file_memory, int is_root, uint16_t start_cluster){
        //local variables
        char *pseudo_input_string=strdup(input_string);
        char* pseudo_file_name=strdup(file_name);
        char delimiter[2]="/";
        char* token=strtok(pseudo_input_string,delimiter);

        
        char *new_string= input_string + strlen(token)+1;


    //temporary_pointer servers a pointer that points along the FAT system. It starts at the boot sector but is adjusted to start at either the root dir or the following data area
	char* temporary_pointer = is_root ? (file_memory + (boot_info.bytes_per_sector * 19)) : (file_memory + (33+start_cluster - 2) * boot_info.bytes_per_sector);

                      

        //loop to find the directory that the file is located in              
        while(temporary_pointer[0]!=0x00){

                if((temporary_pointer[11]&0x10)!=0x10){

                    char read_file[9];
                    strncpy(read_file,(char*)(temporary_pointer),8);
				read_file[8]='\0';

                    char read_extension 
                    if(strcmp(file_name,read_file)==0){
                        return 69;
                    }
                }
                //check to ensure that it is a directory
                 else if((temporary_pointer[11]&0x10)==0x10) {

                     
		    if (temporary_pointer[0] != '.' && temporary_pointer[1]!='.') {

                //extracting first_logical_cluster of the directory
                uint16_t first_cluster = ((unsigned char)temporary_pointer[26]) | ((unsigned char)(temporary_pointer[27] << 8));
				char read_file[9];

				strncpy(read_file,(char*)(temporary_pointer),8);
				read_file[8]='\0';

                 if(strcmp(token,file_name)==0 && strcmp(file_name,read_file)==0){
                    return 0;
        }
                


                if(strncmp(read_file,token,strlen(token))==0){
                  uint16_t result = directorytraverser(new_string, file_name, file_memory, 0, first_cluster);

                    //if the specific file is not found in the directory or the directory does not exist return 0
                    if (result != 0) {
                        free(pseudo_input_string);
                        free(pseudo_file_name);
                        return result;
                    }

                }

			}
	
        }
                temporary_pointer+=32;
        }

        //return the first logical cluster of the directory that the file resides in 
         if(strcmp(token,file_name)==0){
            return start_cluster;
        }

    free(pseudo_input_string);
    free(pseudo_file_name);
    return 0;
}




//function to obtain the next empty fat index in the fat table, the first two indexes are reserved, thus why the offset is necessary
int getNextEmptyFatIndex(char *file_memory){
    uint16_t entry;
	int count=2;

	while((entry=getFatEntry(count,file_memory))!=0x00){
		count++;
	}
	return count;
}


// function to insert details of directory entries
void fileSignaturemaker(char* file_name,int fileSize,int first_cluster, int empty_first_cluster, char* file_memory){

    //checks to see if the file is located in the root directory or if it's located in the data area or any of the subdirs
    char* temporary_pointer; 
    if(first_cluster==19){
        temporary_pointer=(file_memory+(boot_info.bytes_per_sector*first_cluster));
    }
    else{
        temporary_pointer=(file_memory + (33+first_cluster - 2) * boot_info.bytes_per_sector);

    }

   //keep going until it finds an empty directory entry that it can store its information and meta data in 
    while(temporary_pointer[0]!=0x00){
            temporary_pointer+=32;
    }


    //parsing procedure to extract file name and extension

    int index;
    int dot_location = get_last_dot_index(file_name);

    for (index = 0; index < 8; index++) {
        char charac = (index < dot_location) ? file_name[index] : ' ';
        temporary_pointer[index] = charac;
    }

    for (index = 0; index < 3; index++) {
        temporary_pointer[index + 8] = (dot_location != -1 && dot_location + 1 + index < strlen(file_name)) ? file_name[dot_location + 1 + index] : ' ';
    }
        

    //meta data settings
    
    //directory entry attribute
    temporary_pointer[11] = 0x00;

    //directory creation time and other time
    time_t current_time = time(NULL);
    struct tm *current = localtime(&current_time);
    int year = current->tm_year + 1900;
    int month = current->tm_mon + 1;
    int day = current->tm_mday;
    int hour = current->tm_hour;
    int minute = current->tm_min;

    uint16_t date = ((year - 1980) << 9) | (month << 5) | day;
    uint16_t time = (hour << 11) | (minute << 5);

    temporary_pointer[16] = date & 0xFF;
    temporary_pointer[17] = (date >> 8) & 0xFF;
    temporary_pointer[14] = time & 0xFF;
    temporary_pointer[15] = (time >> 8) & 0xFF;

    temporary_pointer[22]= time & 0xFF;
    temporary_pointer[23] = (time >> 8) & 0xFF; 
    temporary_pointer[24] = date & 0xFF;
    temporary_pointer[25] = (date>>8) & 0xFF;
    

    //set up the first logical cluster of the file itself

    temporary_pointer[26] = (unsigned char)(empty_first_cluster & 0xFF);
    temporary_pointer[27] = (unsigned char)((empty_first_cluster >> 8) & 0x0F);

    

    //set up file size

    temporary_pointer[28] = fileSize & 0xFF;
    temporary_pointer[29] = (fileSize >> 8) & 0xFF;
    temporary_pointer[30] = (fileSize >> 16) & 0xFF;
    temporary_pointer[31] = (fileSize >> 24) & 0xFF;



    return;
}


//function to insert the actual data in the file
void dataInserter(char* file_memory, int first_cluster,int file_size, char* inserted_file){

    //local variable 
    char* temporary_pointer=file_memory; //serves as a pointer that moves along the fat12 system's cluster. It always starts at sector 0.
    int bytes_remaining=file_size; 

    //keep loopin until there isn't anything else left to insert
    while(bytes_remaining>0)
    {
        //obtain physical address
        int physical_address= boot_info.bytes_per_sector * (33+first_cluster-2);

        int i;

        for(i=0; i< boot_info.bytes_per_sector;i++){

              if(bytes_remaining==0){
                FatEntryUpdate(first_cluster,temporary_pointer,0xFF);
                return;


            }
            temporary_pointer[physical_address+i]=inserted_file[file_size-bytes_remaining];
            bytes_remaining--;
        }


        //after inserting the data into their respective clusters we have to update the fat table indexes as well to keep 
        // of clusters that it's being pointed or pointing too
        int next_free_empty_cluster=getNextEmptyFatIndex(temporary_pointer);
        FatEntryUpdate(first_cluster,temporary_pointer,next_free_empty_cluster);
        next_free_empty_cluster=getNextEmptyFatIndex(temporary_pointer);
        FatEntryUpdate(first_cluster,temporary_pointer,next_free_empty_cluster);

        first_cluster=next_free_empty_cluster;


    }



}
