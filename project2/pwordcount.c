
//
// Created by Armin Khayyer on 1/31/20.
//
// This is the main function, both parents and child process are implemented inside this main function
// the only argument that needs to be passed to the main function is the filename.

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include "loader.c"
#include "counter.c"
#include <stdbool.h>
#include <signal.h>

#define MAXBUFLEN 5000000
#define READ_END 0
#define WRITE_END 1


//char Loader(char *filename);
int main(int argc, char ** argv)
{
    int num_words = 0;
    char  *text ;
//    char filename[] = "..//input.txt";
    char * filename = argv[1] ;
    char read_msg[MAXBUFLEN];
    int read_msg2;
    pid_t pid;
    int fd[2];
    int fd2[2];

    /* argc should be 3 for correct execution */
    if ( argc != 2 ) {
        /* We print argv[0] assuming it is the program name */
        printf("Usage: ./pwordcount <file_name> \n");
        return 0;
    }

    /* create the pipe  and check if they are working fine*/
    if ((pipe(fd) == -1) || (pipe(fd2) == -1) )
    {
        fprintf(stderr,"one of the Pipes failed");
        return 1; }
    /* now fork a child process */
    pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "Fork failed");
        return 1; }
    if (pid > 0)
    {  /* parent process */ /* close the unused end of the pipe */
        close(fd[READ_END]);

        text = Loader(filename);
        if(text == "1"){
            kill(pid, SIGKILL);
            exit(1);
        }

        printf("Process 1 starts sending data to Process 2 ...\n");
        /* write to the pipe */
        write(fd[WRITE_END], text, strlen(text)+1);
        /* close the write end of the pipe */
        close(fd[WRITE_END]);
        int returnStatus;
        waitpid(pid, &returnStatus, 0);
        close(fd2[WRITE_END]);
        /* read from the pipe */
        read(fd2[READ_END], &read_msg2, 100 );
        printf("Process 1: The total number of words is %d. \n", read_msg2);

    }
    else {
        /* child process */
        /* close the unused end of the pipe */
        close(fd[WRITE_END]);
        /* read from the pipe */
        read(fd[READ_END], read_msg, MAXBUFLEN );
        printf("Process 2 finishes receiving data from Process 1 ...\n");
        num_words = counter(read_msg);

        close(fd2[READ_END]);
        printf("Process 2 is sending the result back to Process 1 ...\n");
        write(fd2[WRITE_END], &num_words, strlen(read_msg)+1);
        close(fd2[WRITE_END]);
        /* close the write end of the pipe */
        close(fd[READ_END]);
        }
    return 0; }
