/*
 * COMP7500/7506
 * Project 3: commandline_parser.c
 *
 * This sample source code demonstrates how to:
 * (1) separate policies from a mechanism
 * (2) parse a commandline using getline() and strtok_r()
 *
 * The sample code was derived from menu.c in the OS/161 project
 *
 * Xiao Qin
 * Department of Computer Science and Software Engineering
 * Auburn University
 * Modified by Armin Khayyer
 * Auburn University
 *
 * Compilation Instruction:
 * gcc commandline_parser.c -o commandline_parser
 * ./commandline_parser
 *
 */

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "QueueHeader.h"
#include <time.h>
#include <sys/time.h>

#define NUM_OF_CMD   5  /* The number of submitted jobs   */
#define queue_SIZE 200
/*
 * This function simulates a terminal where users may
 * submit jobs into a batch processing queue.
 * Note: The input parameter (i.e., *ptr) is optional.
 * If you intend to create a thread from a function
 * with input parameters, please follow this example.
 */


extern u_int queue_head;
extern u_int queue_tail;
extern u_int job_num;
extern u_int job_finished;
extern u_int test;

extern pthread_mutex_t queue_lock;
extern pthread_cond_t cmd_buf_not_full;
extern pthread_cond_t cmd_buf_not_empty;
struct Queue JobQueue[queue_SIZE];
struct FInishQueue FinishJobQueue[2*queue_SIZE];
struct Queue Runing_job;
struct Metrics metrics ;

/* Error Code */
#define EINVAL       1
#define E2BIG        2

#define MAXMENUARGS  10
#define MAXCMDLINE   64

char *policy = "fcfs";

void menu_execute(char *line, int isargs);
int cmd_run(int nargs, char **args);
int cmd_quit(int nargs, char **args);
void showmenu(const char *name, const char *x[]);
int cmd_helpmenu(int n, char **a);
int cmd_dispatch(char *cmd);
void *Scheduler();
int delete_job(int tail, int head);


/*
 * The run command - submit a job.
 */

//the following three functions are used as comparator for qsort function to sort the job queue
int comparator_priority(const void* p, const void* q)
{
    if (((struct Queue*)q)->Priority ==
        ((struct Queue*)p)->Priority){
        return ((struct Queue*)p)->start.tv_sec -
               ((struct Queue*)q)->start.tv_sec;
    }
    return ((struct Queue*)q)->Priority -
           ((struct Queue*)p)->Priority;
}

int cmd_donothing(int nargs, char **args) {
    return 0;
}

int comparator_sjf(const void* p, const void* q)
{
    if (10000*((struct Queue*)p)->Time ==
        10000*((struct Queue*)q)->Time)
    {

        return ((struct Queue*)p)->start.tv_sec -
               ((struct Queue*)q)->start.tv_sec;
    }
    return 10000*((struct Queue*)p)->Time -
            10000*((struct Queue*)q)->Time;
}

int comparator_fcfs(const void* p, const void* q)
{
    return ((struct Queue*)p)->start.tv_sec -
           ((struct Queue*)q)->start.tv_sec;
}

//this function is used to delete a dispatched job from the queue
int delete_job(int tail, int head) {
    for (int i = tail + 1; i < head + 1; i++ ) {
        strcpy(JobQueue[i-1].job_name, JobQueue[i].job_name);
        JobQueue[i-1].Time = JobQueue[i].Time;
        JobQueue[i-1].Priority = JobQueue[i].Priority;
        JobQueue[i-1].start = JobQueue[i].start;
    }
    return 0; /* if succeed */
}


int cmd_run(int nargs, char **args) {
    if (nargs <4 ) {
        printf("Usage: run <job> <time> <priority>\n");
        return EINVAL;
    }
    pthread_mutex_lock(&queue_lock);
    while (job_num == queue_SIZE ) {
        pthread_cond_wait(&cmd_buf_not_full, &queue_lock);
    }
    strcpy(JobQueue[queue_head].job_name, args[1]);
    printf("Job %s was submitted.\n",args[1] );
    JobQueue[queue_head].Time = atof(args[2]);
    JobQueue[queue_head].Priority = atoi(args[3]);
//    time_t now =
//    JobQueue[queue_head].arrival_time = time(&now);
    struct timeval start;
//    gettimeofday(&start, NULL);
    gettimeofday(&start, NULL);
    JobQueue[queue_head].start = start;
//    JobQueue[queue_head].time_innano;
//    printf("time taken %f\n",start);
    job_num ++;
    queue_head ++;
    printf("Total number of jobs in the queue: %d\n",job_num);
    pthread_mutex_unlock(&queue_lock);
    Scheduler();
    pthread_mutex_lock(&queue_lock);
//    float waiting_time = 0;
//    for (int i = queue_tail; i < queue_head + 1; i++) {
//        JobQueue[i].Expected_wait_time = waiting_time;
//        waiting_time += JobQueue[i].Time;
//        i++;
//    }
    int index = 0;
    for (int i= 0; i < job_num ; i++){
        if((strcmp(JobQueue[i].job_name, args[1]) == 0) && (JobQueue[i].Time ==atof(args[2])) &&
        JobQueue[i].start.tv_sec == start.tv_sec )
        {
            index = i;
            break;
        }
    }

    float waiting_time = Runing_job.Time;
    for (int j = 0; j < index; j++) {
        waiting_time += JobQueue[j].Time;
    }
    printf("Expected waiting time: %f seconds.\n",  waiting_time);
    printf("Scheduling Policy: %s.\n", policy);
    pthread_mutex_unlock(&queue_lock);
    pthread_cond_signal(&cmd_buf_not_empty);
    /* Use execv to run the submitted job in AUbatch */
    return 0; /* if succeed */
}

//
int help_test(int nargs, char **args) {
    if (strstr(args[1], "-test") != NULL){
        printf("test <benchmark> <policy> <num_of_jobs> <arrival_rate> <priority_levels>\n");
        printf("     <min_CPU_time><max_CPU_time>\n");
    } else{
        printf("Not a valid flag, please try again.\n");
    }

}

int cmd_test(int nargs, char **args){
    if (nargs <8 ) {
        printf("Usage: test <benchmark> <policy> <num_of_jobs> <arrival_rate> <priority_levels> <min_CPU_time> <max_CPU_time>\n");
        return EINVAL;
    }
    if(job_num !=0  ){
        printf("\033[1;31m");
        printf("Warning, There are already some jobs in the queue, please wait for all the jobs in queue to be done\n");
        printf("\033[0m");
        return 0;
    } else if( job_finished != 0 ) {
        char res[2];
        printf("\033[1;31m");
        printf("Warning");
        printf(", We have recorded the metrics of interest for the jobs you have entered before.\nDo you wish to reset all the metrics to make sure you get correct results for your recent test case? (Y/N)\n");
        printf("\033[0m");
        scanf("%s",res);
        if ((strcmp(res, "Y") == 0)||(strcmp(res, "y") == 0)){
            printf("All the metrics are reset to zero\n");
            job_finished = 0;
        }
    }

    int NumberOfJobs = atoi(args[3]);
    test = NumberOfJobs;
    float arr_rate = atof(args[4]);
    float IAT = 1/arr_rate;
    float min = atof(args[6]);
    float max = atof(args[7]);
    int priority = atoi(args[5]);
    for (int i =0; i<NumberOfJobs; i++){
        pthread_mutex_lock(&queue_lock);
        while (job_num == queue_SIZE ) {
            pthread_cond_wait(&cmd_buf_not_full, &queue_lock);
        }
        policy = args[2];
        double cpu_time = min + ((double)rand()/RAND_MAX * (max-min));
        int pri = rand() % (priority+1) + 1;
        char name[20];
        sprintf(name, "job %d", i);
        strcpy(JobQueue[queue_head].job_name, name);
        JobQueue[queue_head].Time = cpu_time;
        JobQueue[queue_head].Priority = pri;
        struct timeval arr_time;
        gettimeofday(&arr_time, NULL);
        JobQueue[queue_head].start = arr_time;
        job_num ++;
        queue_head ++;
        pthread_mutex_unlock(&queue_lock);
        Scheduler();
        pthread_cond_signal(&cmd_buf_not_empty);
        usleep(IAT * 1000000);
    };

    /* Use execv to run the submitted job in AUbatch */
    return 0; /* if succeed */
}

void print_results(){
    printf("\n%-40s %-4d\n", "Total number of job submitted:", job_finished);
    printf("%-40s %-4f %-10s\n", "Average turnaround time:", metrics.Average_turnaround, "seconds.");
    printf("%-40s %-4f %-10s\n", "Average CPU time:", metrics.Average_cpu_time, "seconds.");
    printf("%-40s %-4f %-10s\n", "Average waiting time:", metrics.average_waiting_time, "seconds.");
    printf("%-40s %-4f \n", "Throughput:", metrics.throughput);
    printf("khar");
    FILE *fptr;
    // use appropriate location if you are using MacOS or Linux
    fptr = fopen("Metrics.txt","w");
    if(fptr == NULL)
    {
        printf("Error!");
        exit(1);
    }

    fprintf(fptr,"%-40s %-4d\n", "Total number of job submitted:", job_finished);
    fprintf(fptr,"%-40s %-4s\n", "Policy", policy);
    fprintf(fptr,"%-40s %-4f %-10s\n", "Average turnaround time:", metrics.Average_turnaround, "seconds.");
    fprintf(fptr, "%-40s %-4f %-10s\n", "Average CPU time:", metrics.Average_cpu_time, "seconds.");
    fprintf(fptr,"%-40s %-4f %-10s\n", "Average waiting time:", metrics.average_waiting_time, "seconds.");
    fprintf(fptr,"%-40s %-4f \n", "Throughput:", metrics.throughput);
    fprintf(fptr,"%-40s %-4f \n", "max_response_time :", metrics.max_response_time);
    fprintf(fptr,"%-40s %-4f \n", "min_response_time :", metrics.min_response_time);
    fclose(fptr);
}

/*
 * The quit command.
 */
int cmd_quit(int nargs, char **args) {
    if (nargs != 2 ) {
        printf("Usage: quit <i/d>, where <i> is immediate quiting and <d> waits for all the job in queue to run and then prints the metrics\n");
        return EINVAL;
    }
    char *flag;
    flag = args[1];
//    strcat(flag, "\0");
    if (strstr(flag, "-i") != NULL){
        printf("%-40s %-4d\n", "Total number of job submitted:", job_finished);
        printf("%-40s %-4f %-10s\n", "Average turnaround time:", metrics.Average_turnaround, "seconds.");
        printf("%-40s %-4f %-10s\n", "Average CPU time:", metrics.Average_cpu_time, "seconds.");
        printf("%-40s %-4f %-10s\n", "Average waiting time:", metrics.average_waiting_time, "seconds.");
        printf("%-40s %-4f \n", "Throughput:", metrics.throughput);
        exit(0);
    }
    else if (strstr(flag, "-d") != NULL){
        while((job_num != 0) ||(Runing_job.Time != 0) || (strcmp(Runing_job.job_name,  "\0") != 0) ){
            continue;
        }
        printf("%-40s %-4d\n", "Total number of job submitted:", job_finished);
        printf("%-40s %-4f %-10s\n", "Average turnaround time:", metrics.Average_turnaround, "seconds.");
        printf("%-40s %-4f %-10s\n", "Average CPU time:", metrics.Average_cpu_time, "seconds.");
        printf("%-40s %-4f %-10s\n", "Average waiting time:", metrics.average_waiting_time, "seconds.");
        printf("%-40s %-4f \n", "Throughput:", metrics.throughput);
        exit(0);

    } else{
        printf("%s is not a valid flag for quit function\n", flag);
    }
}
int cmd_fcfs(int nargs, char **args) {
    policy = "fcfs";
    Scheduler();
    printf("Scheduling policy is switched to FCFS. All the %d waiting jobs have been rescheduled. \n", job_num);
}

int cmd_sjf(int nargs, char **args) {
    policy = "sjf";
    Scheduler();
    printf("Scheduling policy is switched to SJF. All the %d waiting jobs have been rescheduled. \n", job_num);
}
int cmd_priority(int nargs, char **args) {
    policy = "priority";
    Scheduler();
    printf("Scheduling policy is switched to Priority. All the %d waiting jobs have been rescheduled. \n", job_num);
}


int cmd_display(int nargs, char **args) {
    char MY_TIME[9];
    struct tm *arr_time ;
    char RUN_time[9];
    struct tm *running_arr_time ;
    printf("Total number of jobs in the queue: %d \n", job_num);
    printf("Scheduling Policy: %s \n", policy);
    printf("%-10s %-10s %-5s  %-15s %-10s\n", "Name", "CPU_time", "pri", "arrival_time", "progress");
    running_arr_time = localtime(&Runing_job.start.tv_sec);
    strftime(RUN_time, sizeof(RUN_time), "%H:%M:%S", running_arr_time);
    if((strcmp(Runing_job.job_name,  "\0") != 0) && (Runing_job.Time != 0)) {
        printf("%-10s %-10f %-5d  %-15s %-10s \n", Runing_job.job_name, Runing_job.Time, Runing_job.Priority, RUN_time,
               "Run");
    }
    pthread_mutex_lock(&queue_lock);
    for (int i = queue_tail; i < queue_head; i++) {
//        time( &JobQueue[i].arrival_time );
        arr_time = localtime(&JobQueue[i].start.tv_sec);
        strftime(MY_TIME, sizeof(MY_TIME), "%H:%M:%S", arr_time);
        printf("%-10s %-10f %-5d  %-15s %-10s \n"
                ,JobQueue[i].job_name, JobQueue[i].Time, JobQueue[i].Priority, MY_TIME , "in_queue");
    }
    pthread_mutex_unlock(&queue_lock);
}


int cmd_display_finish(int nargs, char **args) {
    char Arr_time[9];
    char Frr_time[9];
    char Nano_time[9];
    struct tm *arr_time ;
    struct tm *frr_time ;
    printf("Total number of jobs in the finish queue: %d \n", job_finished);
    printf("Scheduling Policy: %s \n", policy);
    printf("%-10s %-10s %-5s %-15s %-15s %-15s %-15s  \n", "Name", "CPU_time", "pri", "arrival_time", "finish_time", "Turnaround", "Response");
    for (int i = 0; i < job_finished; i++) {
        arr_time = localtime(&FinishJobQueue[i].start.tv_sec);
        strftime(Arr_time, sizeof(Arr_time), "%H:%M:%S", arr_time);

        frr_time = localtime(&FinishJobQueue[i].end.tv_sec);
        strftime(Frr_time, sizeof(Frr_time), "%H:%M:%S", frr_time);
        printf("%-10s %-10f %-5d %-15s %-15s %-15f %-15f\n"
                ,FinishJobQueue[i].job_name, FinishJobQueue[i].Time, FinishJobQueue[i].Priority,Arr_time, Frr_time, FinishJobQueue[i].Turnaround, FinishJobQueue[i].Reponse_time);
    }
}


/*
 * Display menu information
 */
void showmenu(const char *name, const char *x[])
{
    int ct, half, i;

    printf("\n");
    printf("%s\n", name);

    for (i=ct=0; x[i]; i++) {
        ct++;
    }
    half = (ct+1)/2;

    for (i=0; i<half; i++) {
        printf("    %-50s", x[i]);
        if (i+half < ct) {
            printf("%s", x[i+half]);
        }
        printf("\n");
    }

    printf("\n");
}

static const char *helpmenu[] = {
        "[run] <job> <time> <priority> ",
        "[quit] <-i/-d> Exit immediately or delayed ",
        "[help] Print help menu ",
        "[fcfs] change the scheduling policy to fcfs",
        "[sjf] change the scheduling policy to sjf",
        "[priority] change the scheduling policy to priority",
        "[list] display the job status.",
        "[test] test the scheduler with a given benchmark workload information",
        "[Listfinish] display the finished job queue.",

        /* Please add more menu options below */
        NULL
};

int cmd_helpmenu(int n, char **a)
{
    (void)n;
    (void)a;
    showmenu("AUbatch help menu", helpmenu);
    return 0;
}

/*
 *  Command table.
 */
static struct {
    const char *name;
    int (*func)(int nargs, char **args);
} cmdtable[] = {
        /* commands: single command must end with \n */
        { "?\n",	cmd_helpmenu },
        { "h\n",	cmd_helpmenu },
        { "help\n",	cmd_helpmenu },
        { "help",	help_test },
        { "r",		cmd_run },
        { "run",	cmd_run },
        { "q",	cmd_quit },
        { "q\n",	cmd_quit },
        { "quit",	cmd_quit },
        { "SJF\n",	cmd_sjf },
        { "sjf\n",	cmd_sjf },
        { "FCFS\n",	cmd_fcfs },
        { "fcfs\n",	cmd_fcfs },
        { "priority\n",	cmd_priority },
        { "Priority\n",	cmd_priority },
        { "list\n",	cmd_display },
        { "List\n",	cmd_display },
        { "Listfinish\n",	cmd_display_finish },
        { "test",	cmd_test },
        { "Y", cmd_donothing },
        { "y", cmd_donothing },
        { "\0", cmd_donothing },
        { "\n", cmd_donothing },
        /* Please add more operations below. */
        {NULL, NULL}
};

/*
 * Process a single command.
 */
int cmd_dispatch(char *cmd)
{
    time_t beforesecs, aftersecs, secs;
    u_int32_t beforensecs, afternsecs, nsecs;
    char *args[MAXMENUARGS];
    int nargs=0;
    char *word;
    char *context;
    int i, result;

    for (word = strtok_r(cmd, " ", &context);
         word != NULL;
         word = strtok_r(NULL, " ", &context)) {

        if (nargs >= MAXMENUARGS) {
            printf("Command line has too many words\n");
            return E2BIG;
        }
        args[nargs++] = word;
    }

    if (nargs==0) {
        return 0;
    }

    for (i=0; cmdtable[i].name; i++) {
        if (*cmdtable[i].name && !strcmp(args[0], cmdtable[i].name)) {
            assert(cmdtable[i].func!=NULL);

            /*Qin: Call function through the cmd_table */
            result = cmdtable[i].func(nargs, args);
            return result;
        }
    }

    printf("%s: Command not found\n", args[0]);
    return EINVAL;
}

/*
 * Command line main loop.
 */
int User_Interface()
{
    char *buffer;
    size_t bufsize = 64;
    buffer = (char*) malloc(bufsize * sizeof(char));
    if (buffer == NULL) {
        perror("Unable to malloc buffer");
        exit(1);
    }
    while (1) {
        printf("> ");
        getline(&buffer, &bufsize, stdin);
        cmd_dispatch(buffer);
    }
    return 0;
}

void *Scheduler() {
    pthread_mutex_lock(&queue_lock);

//    printf("scheduling jobs \n");
//    while (job_num == queue_SIZE ) {
//        pthread_cond_wait(&cmd_buf_not_full, &queue_lock);
//    }
    if(strcmp(policy, "priority") == 0){
        qsort(JobQueue, queue_head, sizeof(struct Queue), comparator_priority);
    }

    if(strcmp(policy, "sjf") == 0){
        qsort(JobQueue, queue_head, sizeof(struct Queue), comparator_sjf);
    }

    if(strcmp(policy, "fcfs") == 0 ){
        qsort(JobQueue, queue_head, sizeof(struct Queue), comparator_fcfs);
    }
    pthread_mutex_unlock(&queue_lock);

}


void *MetricInfo_evaluator() {
    float avg_tA = 0;
    float avg_cpu = 0;
    float avg_waiting = 0;
    float max_response = FinishJobQueue[0].Reponse_time ;
    float min_response = FinishJobQueue[0].Reponse_time ;
    for (int i = 0; i<job_finished; i++){
        avg_tA += (FinishJobQueue[i].Turnaround/job_finished);
        avg_cpu += (FinishJobQueue[i].Time/job_finished);
        avg_waiting += (FinishJobQueue[i].Reponse_time/job_finished);
        if (FinishJobQueue[i].Reponse_time > max_response ){
            max_response = FinishJobQueue[i].Reponse_time;
        }
        if (FinishJobQueue[i].Reponse_time < min_response ){
            min_response = FinishJobQueue[i].Reponse_time;
        }
    }
    metrics.Average_cpu_time = avg_cpu;
    metrics.Average_turnaround = avg_tA;
    metrics.average_waiting_time = avg_waiting;
    metrics.max_response_time = max_response;
    metrics.min_response_time = min_response;
    metrics.throughput = 1/avg_tA;

}

void *executor(void *ptr) {
//    char *message;
    u_int i;
//    message = (char *) ptr;
//    printf("%s \n", message);

    while (1) {
        /* lock and unlock for the shared process queue */
        pthread_mutex_lock(&queue_lock);
        while (job_num == 0) {
            pthread_cond_wait(&cmd_buf_not_empty, &queue_lock);
        }
        /* Run the command scheduled in the queue */
//        printf("In executor: queue[%d] = %s\n", queue_tail, JobQueue[queue_tail].job_name);
        char cputime[100];
        sprintf(cputime ,"%f",JobQueue[queue_tail].Time);
        char *args[] = {"./process", cputime ,NULL };
        pid_t pid;
        switch ((pid = fork()))
        {
            case -1:
                /* Fork() has failed */
                perror("fork");
                break;
            case 0:
                /* This is processed by the child */
                execv(args[0], args);
                puts("Uh oh! If this prints, execv() must have failed");
                exit(EXIT_FAILURE);
                break;
            default:
                //ruunning struct
                strcpy(Runing_job.job_name , JobQueue[queue_tail].job_name);
                Runing_job.Time = JobQueue[queue_tail].Time;
                Runing_job.Priority = JobQueue[queue_tail].Priority;
                Runing_job.start = JobQueue[queue_tail].start;

                //add to finish list
                strcpy(FinishJobQueue[job_finished].job_name , JobQueue[queue_tail].job_name);
                FinishJobQueue[job_finished].Time = JobQueue[queue_tail].Time;
                FinishJobQueue[job_finished].Priority = JobQueue[queue_tail].Priority;
                FinishJobQueue[job_finished].start = JobQueue[queue_tail].start;
                job_num--;
                queue_head--;
                if (job_num > 0) {
                    delete_job(queue_tail, queue_head);
                }
                pthread_cond_signal(&cmd_buf_not_full);
                /* Unlok the shared command queue */
                pthread_mutex_unlock(&queue_lock);
                wait(NULL);
                time_t now;
//                FinishJobQueue[job_finished].Finish_time = time(&now);
                gettimeofday(&FinishJobQueue[job_finished].end, NULL);
                FinishJobQueue[job_finished].Turnaround = (double)(FinishJobQueue[job_finished].end.tv_usec - FinishJobQueue[job_finished].start.tv_usec) / 1000000 + (double)(FinishJobQueue[job_finished].end.tv_sec - FinishJobQueue[job_finished].start.tv_sec);
                FinishJobQueue[job_finished].Reponse_time = FinishJobQueue[job_finished].Turnaround - (double)FinishJobQueue[job_finished].Time;
                job_finished ++;
                MetricInfo_evaluator();
                strcpy(Runing_job.job_name, "\0" );
                Runing_job.Time = 0;
                if (job_finished == test && job_num == 0){
                    print_results();
                }


    }
    }
    /* end for */
}
