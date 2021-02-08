struct Queue{
    char job_name[30];
    float Time;
    int Priority;
    struct timeval start;
};


struct FInishQueue{
    char job_name[30];
    float Time;
    int Priority;
//    time_t arrival_time;
//    time_t Finish_time;
    struct timeval start;
    struct timeval end;
    double Turnaround;
    float Reponse_time;
};


struct Metrics{
    float Average_turnaround;
    float Average_cpu_time;
    float average_waiting_time;
    float throughput;
    float max_response_time;
    float min_response_time;
    float Response_Time_Standard_Deviation;
};