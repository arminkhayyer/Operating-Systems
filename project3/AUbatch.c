/*
 * COMP7500/7506
 * Project 3: AUbatch - A Batch Scheduling System
 *
 * Xiao Qin
 * Department of Computer Science and Software Engineering
 * Auburn University
 * Feb. 20, 2018. Version 1.1
 * Modified By Armin Khayyer on march 2020
 *
 * This sample source code demonstrates the development of
 * a batch-job scheduler using pthread.
 *
 * Compilation Instruction:
 * gcc pthread_sample.c -o pthread_sample -lpthread
 *
 * Learning Objecties:
 * 1. To compile and run a program powered by the pthread library
 * 2. To create two concurrent threads: a scheduling thread and a dispatching thread
 * 3. To execute jobs in the AUbatch system by the dispatching thread
 * 4. To synchronize the two concurrent threads using condition variables
 *
 * How to run aubatch_sample?
 * 1. You need to compile another sample code: process.c
 * 2. The "process" program (see process.c) takes two input arguments
 * from the commandline
 * 3. In aubtach: type ./process 5 10 to submit program "process" as a job.
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>

#include <zconf.h>
#include <unistd.h>
#include "comand.c"

typedef unsigned int u_int;


/*
 * When a job is submitted, the job must be compiled before it
 * is running by the executor thread (see also executor()).
 */
void *executor( void *ptr );    /* To simulate job execution */

pthread_mutex_t queue_lock;  /* Lock for critical sections */
pthread_cond_t cmd_buf_not_full; /* Condition variable for buf_not_full */
pthread_cond_t cmd_buf_not_empty; /* Condition variable for buf_not_empty */

/* Global shared variables */
u_int queue_head;
u_int queue_tail;
u_int job_num;
u_int job_finished;
u_int test;



int main() {
    printf("Welcome to Armin's batch job scheduler Version 1.0\nType ‘help’ to find more about AUbatch commands.\n");
    pthread_t command_thread, executor_thread, UserInterface_thread; /* Two concurrent threads */
//    char *message1 = "Command Thread";
//    char *message2 = "Executor Thread";
//    char *message3 = "User Interface Thread";
    int  iret1, iret2, iret3;

    /* Initilize count, two buffer pionters */
    job_num = 0;
    queue_head = 0;
    queue_tail = 0;
    job_finished = 0;
    test = 0;

    /* Create two independent threads:command and executors */
    iret3 = pthread_create(&UserInterface_thread, NULL, User_Interface, NULL);
//    iret1 = pthread_create(&command_thread, NULL, commandline, (void*) message1);
    iret2 = pthread_create(&executor_thread, NULL, executor, NULL);


    /* Initialize the lock the two condition variables */
    pthread_mutex_init(&queue_lock, NULL);
    pthread_cond_init(&cmd_buf_not_full, NULL);
    pthread_cond_init(&cmd_buf_not_empty, NULL);

    /* Wait till threads are complete before main continues. Unless we  */
    /* wait we run the risk of executing an exit which will terminate   */
    /* the process and all threads before the threads have completed.   */
//    pthread_join(command_thread, NULL);
    pthread_join(UserInterface_thread, NULL);
    pthread_join(executor_thread, NULL);

//
    printf("command_thread returns: %d\n",iret3);
    printf("executor_thread returns: %d\n",iret2);
    exit(0);
}
