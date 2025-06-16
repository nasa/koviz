#ifndef _KOVIZ_H
#define _KOVIZ_H

/*
 * README
 *
 * Instructions for TS21 Trick
 *     % cd SIM_iss
 *     % g++ -I$TRICK_HOME/include -c koviz.c 
 *     % ar cr libkoviz.a koviz.o
 *     % vi S_overrides.mk ; # add line: TRICK_USER_LINK_LIBS += libkoviz.a
 *     % QCP -u ; # try this first to link in libkoviz.a to the sim
 *     % 
 *     % vi <modeldir/modelfile.cpp>
 *          add:
 *          #include <full_path>/koviz.h"
 *          ...
 *          koviz_start("utime","<tag>");
 *          code to profile
 *          koviz_stop("utime","<tag>");
 *
 *     % QCP -u <modeldir/modelfile.cpp>
 *
 *     % run the sim to get koviz.trk
 *     % koviz .
 *
 */

//#define TRICK  // comment out if no Trick
#ifdef TRICK
#include "trick/exec_proto.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

// Things to change (in TS21 I had KOVIZ_NRECS set to 1000000)
#define KOVIZ_MAX_STATS   4 
#define KOVIZ_NRECS       3000

// Things most likely to stay same
#define KOVIZ_CYCLE      1.000
#define KOVIZ_STAT_SIZE  32
#define KOVIZ_TAG_SIZE   96
#define KOVIZ_NAME_SIZE  128
#define KOVIZ_SHUTDOWN_TIME 1000000.0

struct koviz_stat
{
    char stat[KOVIZ_STAT_SIZE];
    char tag[KOVIZ_TAG_SIZE];
    struct rusage rusage_start;
    struct timeval gtod_start;
    long utime;
};

// public
void koviz_start(const char* stat, const char* tag);
void koviz_stop(const char* stat, const char* tag);

// private
int  _koviz_idx(const char* stat, const char* tag);
void _koviz_mmap_trk();
int  _koviz_file_size();
int  _koviz_header_size();
int  _koviz_data_size();
void _koviz_write_header(char* map, int fd);

#ifndef TRICK
double exec_get_sim_time();
void exec_terminate(const char* msg1, const char* msg2);
#endif

#endif
