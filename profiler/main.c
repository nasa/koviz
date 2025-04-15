#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "koviz.h"

extern double kovizTime;

int main()
{
    int i, j, k;
    struct timespec start;
    struct timespec stop;
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 1000000;
    kovizTime = 0.0;
    for ( i = 0; i < KOVIZ_NRECS; ++i ) {
        koviz_start("gtod", "my.frame_time_3000us");

        koviz_start("gtod", "my.usleep(0.001)");
        usleep(1000);
        koviz_stop("gtod", "my.usleep(0.001)");

        koviz_start("gtod", "my.nanosleep(0.001)");
        nanosleep(&ts,NULL);
        koviz_stop("gtod", "my.nanosleep(0.001)");

        koviz_start("gtod", "my.spin(0.001)");
        clock_gettime( CLOCK_REALTIME, &start);
        while (1) {
            clock_gettime( CLOCK_REALTIME, &stop);
            long d = 1e9*(stop.tv_sec-start.tv_sec)+
                          stop.tv_nsec-start.tv_nsec;
            if ( d > 1000000 ) {
                break;
            }
        }
        koviz_stop("gtod", "my.spin(0.001)");
        koviz_stop("gtod", "my.frame_time_3000us");

        kovizTime = kovizTime + 1.0;
    }

    return 0;
}
