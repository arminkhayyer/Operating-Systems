/*
 * This Function gets a string of words as input, counts the number of words inside it, and it will
 * return the number of words once its done.
 */


#include <stdio.h>
#include <string.h>


int counter(char * textstring)
{
    int count = 0;
    int iterator = 0;
    int init_space = 1;
    printf("Process 2 is counting words now ...\n");
    //loop over the characters and increament the count value once a character
    // is seen before which there exists a space.
    for (iterator = 0;iterator<strlen(textstring);iterator++)
    {
        if ((textstring[iterator] == ' ') ||(textstring[iterator] == '\n'))
            init_space = 1;
        else
        {
            if (init_space == 1)
                count++;
            init_space = 0;
        }
    }
    return count;
}