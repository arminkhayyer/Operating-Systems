
/* This file is an implementation of the Loader Function
 * It takes the file name as input, opens the file, read it line by line into memory
 * And finally returns the output.*/


#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#define MAXBUFLEN 5000000


char *Loader(char *filename) {
    FILE *file;
    int lenfilename = strlen(filename);
    char * format = ".txt";

    static char textstring[MAXBUFLEN + 1];
    //check if the filename is at least 4 character long.
    if(lenfilename < 4){
        printf("Usage: ./pwordcount <file_name> \n");
        printf("Please enter a valid file name \n");
        return "1";
    }
    //check if the format of the file is .txt
    if ( strcmp(format+strlen(format)-4, filename+strlen(filename)-4) != 0 ){
        printf("File format should be .txt \n");
        return "1";
    }

    file = fopen(filename, "r" );
    /* fopen returns 0, the NULL pointer, on failure */
    if ( file == 0 ) {
        printf( "Could not open file: %s.\n", filename);
        printf("Usage: ./pwordcount <file_name> \n");
        return "1";
    }
    //read the file into the textstring variable.
    else {
        printf("Process 1 is reading file %s now ...\n", filename);
        fread(textstring, sizeof(char), MAXBUFLEN, file);
        if ( ferror( file ) != 0 ) {
            fputs("Something went wrong during reading the file", stderr);
        }
        strcat(textstring, "\0"); // just to make sure that the text terminates correctly
        fclose( file );
    }
    return textstring;
}
