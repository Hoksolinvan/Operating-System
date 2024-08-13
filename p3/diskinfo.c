#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include "FAT_operation.h"



//function prototypes

void OS_NAME(char *file_memory);
void LABEL_extractor(char *file_memory);
void SECTOR_SIZE_extractor(char *file_memory);
void SECTOR_NUM_extractor(char *file_memory);
int Free_disk_size(char *file_memory);
int file_counter(char* file_memory,int cluster_start, int is_root);
int diskimagechecker(char *str);


//boot sector information
struct boot_sector{
	
	char OS_NAME[9]; //OS_NAME
	uint16_t bytes_per_sector; //TOTAL SIZE OF DISK
	uint8_t num_of_FAT;		//Number of FAT copies
	uint16_t total_sector_count;  //TOTAL SIZE OF DISK
	uint16_t sectors_per_FAT;  //Sectors per FAT
	char volume_label[12]; //LABEL OF DISK




}boot_info;


int main(int argc, char * argv[]){


	//input-validation checks
	if(argc<2){
		printf("No disk image file was inserted!\n");
		return 1;
	}

	int input_validation_1=diskimagechecker(argv[1]);
	

	 if(argc>2){
		
		printf("Please ensure that you know how to use the program properly (Overloaded number of arguments)\n");
		return 1;

	}
		



	//setting up the disk image file and creating a memory mapping, promoting easy manipulation and access of data
	int file_descriptor = open(argv[1],O_RDWR);
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


	//additional check to ensure that the file is a FAT12 system eventhough if it might not have a .ima extension
	char system_type[9];
	char* fat_type="FAT12   ";
	strncpy(system_type,file_memory+54,8);
	system_type[9]='\0';

	if((input_validation_1==0 && strcmp(system_type,fat_type)!=0) ||strcmp(system_type,fat_type)!=0 ){

		printf("Please ensure that you know how to use the program \n");
		return 1;

	}

	//Extracting the bytes_per_sector
	SECTOR_SIZE_extractor(file_memory);


	if(file_info.st_size < boot_info.bytes_per_sector){

		printf("The file is too small to contain a boot sector.Therefore, it isn't a MS-DOS FAT12 File System\n");
		munmap(file_memory,file_info.st_size);
		close(file_descriptor);
		return 1;
	


	}

	//Extracting the OS Name from the Disk Image
	OS_NAME(file_memory);

	

	//Extracting the label of the disk from the Disk Image
	LABEL_extractor(file_memory);

	//Extracting number of sectors
	
	SECTOR_NUM_extractor(file_memory);

	//Extracting the number of FATs
	
	boot_info.num_of_FAT=file_memory[16];

	//Extracting sector per FAT
	
	boot_info.sectors_per_FAT=file_memory[22]+(file_memory[23]<<8);

	printf("\nOS Name: %s \n",boot_info.OS_NAME);
	printf("Label of the disk: %s \n",boot_info.volume_label);
	
	int total_disk_size=boot_info.bytes_per_sector*boot_info.total_sector_count;
	printf("Total size of the disk: %d bytes \n",total_disk_size);

	int free_disk_size=Free_disk_size(file_memory);
	int file_count=file_counter(file_memory,0,1);
	printf("Free size of the disk: %d bytes \n",free_disk_size);

	printf("==============\n");
	printf("The number of files in the disk (including all files in the root directory and files in all subdirectories): %d\n",file_count);
	printf("==============\n");
	printf("Number of FAT copies: %d \n",boot_info.num_of_FAT);
	printf("Sectors per FAT: %d \n\n",boot_info.sectors_per_FAT);
	











	munmap(file_memory,file_info.st_size);
	close(file_descriptor);
	return 0;
}

//function to acquire OS name from boot sector
void OS_NAME(char* file_memory){


strncpy(boot_info.OS_NAME,(char*)(file_memory+3),8);
boot_info.OS_NAME[8]='\0';


}




//function to acquire label from boot sector or dir
void LABEL_extractor(char* file_memory){


//first check to see if the label is located in the boot sector
strncpy(boot_info.volume_label,(char*)(file_memory+43),11);
boot_info.volume_label[11]='\0';


//if it isn't then we look for it in the root directory
if(boot_info.volume_label[0]==' '){
//local variables
char* temporary_pointer=file_memory;
temporary_pointer+=boot_info.bytes_per_sector*19;


//while loop to look for the label
while(temporary_pointer[0]!=0x00){
if(temporary_pointer[11]==0x08){
strncpy(boot_info.volume_label,(char*)(temporary_pointer),11);
boot_info.volume_label[12]='\0';
break;
}
temporary_pointer+=32;
}	
}

return;
}


//function to acquire size of a sector
void SECTOR_SIZE_extractor(char* file_memory){
boot_info.bytes_per_sector=file_memory[11]+(file_memory[12]<<8);
return;
}

//function to acquire count of sectors
void SECTOR_NUM_extractor(char* file_memory){
boot_info.total_sector_count=file_memory[19]+(file_memory[20]<<8);	
return;
}


//function to calculate the number of free disk size
int Free_disk_size(char* file_memory){

	//count variables
	int count=0;
	int fat_entries = ((8*boot_info.sectors_per_FAT*boot_info.bytes_per_sector)/12);
	

	//ignores the first 31 sectors as we are only interested in calculating the data area
	for(int i=2; i<fat_entries;i++){
			
		if((getFatEntry(i,file_memory)==0x000) && ((i+31)<boot_info.total_sector_count)){
			count++;
		}
	
	}

	return boot_info.bytes_per_sector*count;
}


int file_counter(char* file_memory,int cluster_start, int is_root){
int file_count=0;
char* temporary_pointer;

if (is_root==1) {
        temporary_pointer = file_memory+(boot_info.bytes_per_sector * 19);  // Root directory starts at sector 19
    } else {
        temporary_pointer = file_memory + (boot_info.bytes_per_sector*33)+ (cluster_start - 2) * boot_info.bytes_per_sector;
    }


while(temporary_pointer[0]!= 0x00){


if(((temporary_pointer[26]|= temporary_pointer[27]<<8 )&0xff)!=0 && ((temporary_pointer[26]|= temporary_pointer[27]<<8 )&0xff)!=1 && (temporary_pointer[11]&0x10)!=0x10){
file_count++;  
}
else if((temporary_pointer[11]&0x10)==0x10){
if (temporary_pointer[0] != '.' && temporary_pointer[1] != '.') {  // Skip '..' entries

                uint16_t first_cluster = temporary_pointer[26] | (temporary_pointer[27] << 8);
                file_count += file_counter(file_memory, first_cluster, 0);  // Recurse into subdirectory
            
			
			}

}
temporary_pointer+=32;
}

return file_count;
}



//function to check extension type of the disk image
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
