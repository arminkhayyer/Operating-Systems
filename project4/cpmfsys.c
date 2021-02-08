//
// Created by Armin Khayyer on 4/5/20.
//


#include "cpmfsys.h"
#include <stdbool.h>
#include <regex.h>

bool FreeList[NUM_BLOCKS];


//function to allocate memory for a DirStructType (see above), and populate it, given a
//pointer to a buffer of memory holding the contents of disk block 0 (e), and an integer index
// which tells which extent from block zero (extent numbers start with 0) to use to make the
// DirStructType value to return.
DirStructType *mkDirStruct(int index,uint8_t *e){
    /* e points to in-memory block0 */
    DirStructType * dir = malloc(sizeof(DirStructType));
    /* Get the extent address in block0 */
    uint8_t * dir_addr = e + index*EXTENT_SIZE;

    /*copy status from block0*/
    dir->status = dir_addr[0];
    int byte_num;
    for (byte_num = 0; byte_num <= 8; byte_num++) {
        //copy the data from disk to the dir structure name
        if (dir_addr[byte_num+ 1] != ' ' && byte_num <=7) {
            dir->name[byte_num] = (char)dir_addr[byte_num+1];
        } else{
            dir->name[byte_num] = '\0';
            break;
        }
    }

    for (byte_num = 0; byte_num <= 3; byte_num++) {
        //copy the data from disk to the dir structure extension
        if (dir_addr[byte_num + 9] != ' '&& byte_num <=2) {
            dir->extension[byte_num] = (char)dir_addr[byte_num+9];
        } else{
            dir->extension[byte_num] = '\0';
        }
    }
    //copy the data from disk to the dir structure number of blocks
    dir->BC = dir_addr[13];
    dir->RC = dir_addr[15];

    for (byte_num = 0; byte_num <= 15; byte_num++) {
        dir->blocks[byte_num] = dir_addr[byte_num+16];
    }
return dir ;
};



check_if_not_legal(const char *inputstring) {
    static char *legal_pattern = "[^0-9a-zA-Z]";
    //this is the pattern for legal name and extension
    int valid;
    regex_t regex;
    //compilation fails
    if (regcomp(&regex, legal_pattern, REG_NOSUB) != 0) {
        return (0);
    }
    //compare the input string
    valid = regexec(&regex, inputstring, (size_t) 0, NULL, 0);
    regfree(&regex);
    //report error
    if (valid != 0) {
        //return zero for valid names
        return (0);
    }
    //retrun 1 for not valid names
    return (1);
}

// internal function, returns true for legal name (8.3 format), false for illegal
// (name or extension too long, name blank, or  illegal characters in name or extension)
bool checkLegalName(char *name){
    bool islegal = true;
    int lenght = strlen(name);
    if (lenght==0 || lenght>=13){
        islegal = false;
        return islegal;
    }
    char filename[9];
    char extension[4];
    int dot_index = lenght;
    int charac;
    for (charac=0; charac<lenght; charac++){
    if(charac<=7 && name[charac] != '.'){
        filename[charac]=name[charac];
    }
    else{
     if(name[charac] == '.')
     {
         dot_index = charac;
         break;
     } else
         {
            islegal = false;
//          printf("%s", islegal?"true":"false");
            return islegal;
     }
    }
    }
    filename[dot_index] = '\0';
    if (strlen(filename) ==0 )
    {
        islegal = false;    /* check for valid filename*/
//        printf("%s", islegal?"true":"false");
        return islegal;
    }
    if(check_if_not_legal(filename)==1)
    {
        islegal = false;    /* check for valid filename*/
//        printf("%s", islegal?"true":"false");
        return islegal;
    }
    int extension_index = 0;
    if (dot_index < lenght){
        int ext_char;
        for ( ext_char=dot_index+1;ext_char<lenght;ext_char++){
                extension[extension_index] = name[ext_char];
                extension_index ++;
            }
        extension[strlen(extension)-1] = '\0';
        if (strlen(extension) >4){
            islegal = false;    /* check for valid filename*/
//            printf("%s", islegal?"true":"false");
            return islegal;
        }

        if(check_if_not_legal(extension)==1)
        {
            islegal = false;    /* check for valid filename*/
//            printf("%s", islegal?"true":"false");
            return islegal;
        }
        } else if(dot_index == lenght){
        extension[extension_index] = '\0';
    }
//    printf("%s.%s,", filename, extension);
//    printf("%s", islegal?"true":"false");
    return islegal;
    }


// function to write contents of a DirStructType struct back to the specified index of the extent
// in block of memory (disk block 0) pointed to by e
void writeDirStruct(DirStructType *d, uint8_t index, uint8_t *e) {
    uint8_t *dir_addr = e + index * EXTENT_SIZE;
    //overwrite the structure d into block 0
    dir_addr[0] = d->status;
    // make sure that the empty bytes are replaced with white space

    int name_space = 1;
    int byte_num;
    for(byte_num=0;byte_num<8; byte_num++){
        if (d->name[byte_num] != '\0' && name_space==1){
        dir_addr[byte_num+1] = d->name[byte_num];
        } else{
            name_space = 0;
            dir_addr[byte_num+1] = ' ';
        }
    }
    // make sure that the empty bytes in extension are replaced with white space
    int extension_space = 1;
    for( byte_num=0;byte_num<3; byte_num++) {
        if (d->extension[byte_num] != '\0' && extension_space == 1) {
            dir_addr[byte_num + 9] = d->extension[byte_num];
        } else {
            extension_space = 0;
            dir_addr[byte_num + 9] = ' ';
        }
    }
    dir_addr[13] = d->BC;
    dir_addr[15] = d->RC;
    for ( byte_num = 0; byte_num <= 15; byte_num++) {
        dir_addr[byte_num+16] = d->blocks[byte_num];
    }
    uint8_t blockNum = 0;
    //write the block back into the disk
    blockWrite(e,blockNum);
}

// print the file directory to stdout. Each filename should be printed on its own line,
// with the file size, in base 10, following the name and extension, with one space between
// the extension and the size. If a file does not have an extension it is acceptable to print
// the dot anyway, e.g. "myfile. 234" would indicate a file whose name was myfile, with no
// extension and a size of 234 bytes. This function returns no error codes, since it should
// never fail unless something is seriously wrong with the disk
void cpmDir() {
    uint8_t cpm_block0[BLOCK_SIZE];
    uint8_t block0_num = 0;
    blockRead(cpm_block0, block0_num); /*read block0 into cpm*/
    printf("DIRECTORY LISTING\n");
    int i;
    for ( i = 0; i <= 31; i++) {
        DirStructType *cpm_dir = mkDirStruct(i, cpm_block0);
        if (cpm_dir->status != 0xE5) {
//            /* 0xe5 */
//            /* Compute file length */
            int block_number = 0;
            int b_index;
            for ( b_index = 0; b_index < 16; b_index++) {
                if (cpm_dir->blocks[b_index] != 0) {
                    block_number++;
                }
            }

            uint8_t RC = cpm_dir->RC;
            uint8_t BC = cpm_dir->BC;

            int file_length = (block_number - 1) * 1024 + RC * 128 + BC;
            printf("%s.%s %d\n", cpm_dir->name, cpm_dir->extension, file_length);
        }
    }
};

// populate the FreeList global data structure. freeList[i] == true means
// that block i of the disk is free. block zero is never free, since it holds
// the directory. freeList[i] == false means the block is in use.
void makeFreeList(){
    FreeList[0] = false;
    int block;
    for ( block = 1; block<= NUM_BLOCKS-1; block++){
        FreeList[block] = true;
    }
    uint8_t cpm_block0[BLOCK_SIZE];
    uint8_t block0_num = 0;
    blockRead(cpm_block0, block0_num); /*read block0 into cpm*/
//    printBlock(block0_num);
    int i;
    for ( i = 0; i <= 31; i++) {
        DirStructType *cpm_dir = mkDirStruct(i, cpm_block0);
        if (cpm_dir->status != 0xE5) {
            int b_index;
            for ( b_index = 0; b_index < 16; b_index++) {
                if (cpm_dir->blocks[b_index] != 0) {
                    FreeList[cpm_dir->blocks[b_index]] = false;
                }
                }
            }
        }
};


// debugging function, print out the contents of the free list in 16 rows of 16, with each
// row prefixed by the 2-digit hex address of the first block in that row. Denote a used
// block with a *, a free block with a .
void printFreeList(){

    // first make free list
    makeFreeList();
    fprintf(stdout,"FREE BLOCK LIST: (* means in-use)\n");
    int i;
    for ( i = 0; i < NUM_BLOCKS; i++) {
        if (i % 16 == 0) {
            fprintf(stdout,"%2x: ",i);
        }
        if(FreeList[i] == false) {
            fprintf(stdout, "* ");
        } else{
            fprintf(stdout, ". ");
        }
        if (i % 16 == 15) {
            fprintf(stdout,"\n");
        }
    }
} ;

// internal function, returns -1 for illegal name or name not found
// otherwise returns extent nunber 0-31
int findExtentWithName(char *name, uint8_t *block0){
    int legal= checkLegalName(name);
    if (legal == 1)
    {
        char * O_name = strtok(name, ".");
        char * O_extension = strtok(NULL, ".");
        char extension[4];
        if (O_extension == NULL){
            O_extension = extension;
        }
        int extent;
        for ( extent =0; extent<32; extent++){
            DirStructType *cpm_dir = mkDirStruct(extent, block0);
            if (cpm_dir->status != 0xE5) {
                // check if the name matches any extent and return the extent number
                if ((strcmp(cpm_dir->name,O_name) == 0) &&(cpm_dir->status != 0xE5)) {
                    if (strcmp(cpm_dir->extension,O_extension) == 0){
                        return extent;
                    }
                    else {
                        if(O_extension == NULL){return extent;}
                    }
            }
        }}
            return -1;
    }
    if (legal == 0){ return -1;}
};

//read directory block,
// modify the extent for file named oldName with newName, and write to the disk
int cpmRename(char *oldName, char * newName){
    if (checkLegalName(newName) == true)
    {
        char Name_rename[13];
        strcpy(Name_rename, oldName);


        //split the old  name to filename and extension
        char old_name[13];
        strcpy(old_name, oldName);
        char * O_name = strtok(old_name, ".");
        char * O_extension = strtok(NULL, ".");

        //split the new name to filename and extension
        char new_name[13];
        strcpy(new_name, newName);
        char * N_name = strtok(new_name, ".");
        char * N_extension = strtok(NULL, ".");


        uint8_t cpm_block0[BLOCK_SIZE];
        uint8_t block0_num = 0;
        blockRead(cpm_block0, block0_num);
        int extent_num = findExtentWithName(Name_rename, cpm_block0 );
        //if extent number is found replace the old name with the new name in that extent
        if (extent_num != -1)
        {
            DirStructType *cpm_dir = mkDirStruct(extent_num, cpm_block0);
            strcpy(cpm_dir->name, N_name);
            strcpy(cpm_dir->extension, N_extension);
            writeDirStruct(cpm_dir, extent_num, cpm_block0);
            return 0;
        } else{
            return  -1;
        }
    } else{
        return  -1;
    }
};


// delete the file named name, and free its disk blocks in the free list
int  cpmDelete(char * name){
    if (checkLegalName(name) == true) {
        char Name_extent[13];
        strcpy(Name_extent, name);

        char name_del[13];
        strcpy(name_del, name);

        char *D_name = strtok(name_del, ".");
        char *D_extension = strtok(NULL, ".");

        uint8_t cpm_block0[BLOCK_SIZE];
        uint8_t block0_num = 0;
        blockRead(cpm_block0, block0_num);
        int extent_num = findExtentWithName(Name_extent, cpm_block0);
        //if extent number is found delete the information in that extent
        if(extent_num != -1) {
            DirStructType *cpm_dir = mkDirStruct(extent_num, cpm_block0);
            cpm_dir->status = 0xE5;
            strcpy(cpm_dir->name, "\0");
            strcpy(cpm_dir->extension, "\0");
            int byte_num;
            for ( byte_num = 0; byte_num < 16; byte_num++) {
                cpm_dir->blocks[byte_num] = 0;
            }
            //update the free list
            makeFreeList();
            //write the dirstructure in that extent
            writeDirStruct(cpm_dir, extent_num, cpm_block0);
        }
    } else{
        return -1;
    }

};