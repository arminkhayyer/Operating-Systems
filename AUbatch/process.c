//
// Created by Armin Khayyer on 3/1/20.
//

#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>

int main(int argc, char *argv[] )
{
    float cpu_time = atof(argv[1]);
    usleep(cpu_time * 1000000);
    return 0;
}
