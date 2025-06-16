#include "koviz.h"

int   kovizIsFirst = 1;
char* kovizMap = 0;
int   kovizFD  = 0;
int   kovizHeaderSize = 0;
int   kovizRecordSize = 0;
int   kovizCurrRec = 0;
double kovizTime = 0.0;
struct koviz_stat koviz_stats[KOVIZ_MAX_STATS];

#ifndef TRICK
double exec_get_sim_time()
{
    return kovizTime;
}

void exec_terminate(const char* msg1, const char* msg2)
{
    fprintf(stderr,"koviz [terminate]: %s %s\n", msg1, msg2);
    exit(0);
}
#endif

void koviz_start(const char* stat, const char* tag)
{
    if ( kovizCurrRec >= KOVIZ_NRECS ) {
        return;
    }

    if ( kovizIsFirst ) {
        kovizIsFirst = 0;
        int i = 0;
        for ( i = 0; i < KOVIZ_MAX_STATS; ++i ) {
            koviz_stats[i].stat[0] = '\0';
            koviz_stats[i].tag[0] = '\0';
            koviz_stats[i].utime = 0;
        }
        _koviz_mmap_trk();
        _koviz_write_header(kovizMap,kovizFD);
        kovizHeaderSize = _koviz_header_size();
    }

    int idx = _koviz_idx(stat,tag);

    if ( idx == -1 ) {
        // Add new stat (and set idx to added stat)
        int i;
        for ( i = 0; i < KOVIZ_MAX_STATS; ++i ) {
            if ( koviz_stats[i].stat[0] == '\0' ) {
                idx = i;
                strcpy(koviz_stats[idx].stat,stat);
                strcpy(koviz_stats[idx].tag,tag);
                char buf[256];
                buf[0] = '\0';
                strcat(buf,stat);
                strcat(buf,".");
                strcat(buf,tag);
                int statSize = 0;
                statSize += sizeof(int) + KOVIZ_NAME_SIZE ; // name
                statSize += sizeof(int) + strlen("us");     // unit (us for utime)
                statSize += sizeof(int) ;                   // type (TRICK_O7_LONG===7)
                statSize += sizeof(int) ;                   // type size (sizeof(long))
                int j = 48 + statSize*i + 4;  // index of next param (see _koviz_header_size())
                memcpy(&kovizMap[j],buf,strlen(buf));
                break;
            }
        }
        if ( i == KOVIZ_MAX_STATS ) {
            fprintf(stderr,"koviz_start [error]: Too many stats (%d). Aborting!!!\n", KOVIZ_MAX_STATS);
            exit(-1);
        }
    }

    if ( !strcmp(stat,"gtod") ) {
        gettimeofday(&koviz_stats[idx].gtod_start,NULL); 
    } else if ( !strcmp(stat,"utime") ) {
        getrusage(RUSAGE_SELF,&koviz_stats[idx].rusage_start); 
    } else {
        fprintf(stderr,"koviz [error]: stat=\"%s\" not supported... "
                       "e.g. try koviz_start(\"gtod\",\"my.tag\")\n", stat);
        exit(-1);
    }
}

void koviz_stop(const char* stat, const char* tag)
{
    // Allow one pass where stop is before start
    /*
    static int i = 0; 
    if (  kovizIsFirst ) {
        if ( i != 0 ) {
            fprintf(stderr, "koviz_stop [bad scoobs]:  koviz_stop() before koviz_start() !!!\n");
            fprintf(stderr, "                          And no loop to get start before stop\n");
            exit(-1);
        }
        ++i;
        return;
    } 
    */

    struct rusage rusage_stop;
    struct timeval gtod_stop;
    if ( !strcmp(stat,"gtod") ) {
        gettimeofday(&gtod_stop,NULL); 
    } else if ( !strcmp(stat,"utime") ) {
        getrusage(RUSAGE_SELF,&rusage_stop); 
    } else {
        fprintf(stderr,"koviz [error]: stat=\"%s\" not supported... "
                       "e.g. try koviz_stop(\"gtod\",\"my.tag\")\n", stat);
        exit(-1);
    }

    if ( kovizCurrRec >= KOVIZ_NRECS ) {
        return;
    }

    int idx = _koviz_idx(stat,tag);
    if ( idx == -1 ) {
        // If in a loop, koviz_stop() can be before koviz_start()
        // however, it can be an error, so issue warning.  If the
        // tags do not match the warnings will flood the screen.
        fprintf(stderr, "koviz_stop [warning|error]: could not find stat=\"%s\" "
                        "tag=\"%s\".\n", stat, tag);
        fprintf(stderr,"Possible causes: \n");
        fprintf(stderr,"  - koviz_stop tag does not match koviz_start tag\n");
        fprintf(stderr,"  - koviz_stop was called without koviz_start being called first\n");
        return;
    }

    long   j = kovizHeaderSize + kovizCurrRec*kovizRecordSize;
    double t = *((double*)&kovizMap[j]);
    double sim_time = exec_get_sim_time();
    long   utime_elapsed;
    if ( !strcmp(stat,"utime") ) {
        utime_elapsed = 1000000*(rusage_stop.ru_utime.tv_sec  - koviz_stats[idx].rusage_start.ru_utime.tv_sec) +
                                (rusage_stop.ru_utime.tv_usec - koviz_stats[idx].rusage_start.ru_utime.tv_usec);
    } else if ( !strcmp(stat,"gtod") ) {
        utime_elapsed = 1000000*(gtod_stop.tv_sec  - koviz_stats[idx].gtod_start.tv_sec) +
                                (gtod_stop.tv_usec - koviz_stats[idx].gtod_start.tv_usec);
    }

    if ( sim_time > t ) {
        // Record and reset utime
        int i;
        for ( i = 0; i < KOVIZ_MAX_STATS; ++i ) {
            long j = kovizHeaderSize + kovizCurrRec*kovizRecordSize + sizeof(double) + i*sizeof(long);
            memcpy(&kovizMap[j],&koviz_stats[i].utime,sizeof(long));
            if ( i == idx ) {
                koviz_stats[i].utime = utime_elapsed;
            } else {
                koviz_stats[i].utime = 0;
            }
        }

        // Advance to next record
        ++kovizCurrRec;

        // Record sim time
        if ( kovizCurrRec < KOVIZ_NRECS ) {
            long j = kovizHeaderSize + kovizCurrRec*kovizRecordSize;
            memcpy(&kovizMap[j],&sim_time,sizeof(double));
        }
    } else if ( sim_time == t ) {

        // Sum utime when multiple calls per timestamp
        koviz_stats[idx].utime += utime_elapsed;

        // Record utime
        int i;
        for ( i = 0; i < KOVIZ_MAX_STATS; ++i ) {
            long j = kovizHeaderSize + kovizCurrRec*kovizRecordSize + sizeof(double) + i*sizeof(long);
            memcpy(&kovizMap[j],&koviz_stats[i].utime,sizeof(long));
        }
    } else {
        fprintf(stderr,"koviz [bad scoobs]: sim_time=%.9lf < t=%.9lf !!!!\n",sim_time,t);
        exit(-1);
    }

    if ( sim_time >= KOVIZ_SHUTDOWN_TIME ) {

        // Fill rest of trk with timestamps
        int ii = 0;
        int jj = 1;
	double dt = KOVIZ_CYCLE;
        double tt = sim_time+dt;
        for ( ii = kovizCurrRec+1 ; ii < KOVIZ_NRECS; ++ii ) {
            long j = kovizHeaderSize + ii*kovizRecordSize;
            tt = sim_time + jj*dt;
            memcpy(&kovizMap[j],&tt,sizeof(double));
            ++jj;
        }

        if ( msync(kovizMap,_koviz_file_size(),MS_SYNC) == -1 ) {
            fprintf(stderr,"koviz [error]: could not msync mmap to disk!\n");
            exit(-1);
        }
    
        if ( munmap(kovizMap,_koviz_file_size()) == -1 )  {
            fprintf(stderr,"koviz [error]: could not munmap mmaped trk!\n");
            exit(-1);
        }
        close(kovizFD);

        char buf[256];
        sprintf(buf,"shutting down at %g seconds.\n", KOVIZ_SHUTDOWN_TIME);
        exec_terminate(__FILE__, buf);
    }
}

int _koviz_idx(const char* stat, const char* tag)
{
    int idx = -1;

    if ( stat[0] == '\0' || tag[0] == '\0' ) {
        fprintf(stderr,"koviz [error]: stat or tag is null.  Aborting!!!\n");
        exit(-1);
    }

    int i;
    for ( i = 0; i < KOVIZ_MAX_STATS; ++i ) {

        if ( koviz_stats[i].stat[0] == '\0' ) {
            break; // not found
        }

        if ( !strcmp(koviz_stats[i].tag,tag) && 
             !strcmp(koviz_stats[i].stat,stat) ) {
            idx = i;
            break;
        }
    }
    if ( i == KOVIZ_MAX_STATS ) {
        fprintf(stderr,"koviz [error]: Too many stats (%d).  Aborting!!!\n", 
                KOVIZ_MAX_STATS);
        exit(-1);
    }

    return idx;
}

void _koviz_mmap_trk()
{
    const char* fname = "koviz.trk";

    kovizFD = open(fname, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    if ( kovizFD == -1 ) {
        fprintf(stderr, "koviz [error]: could not open file=\"%\"\n",fname);
        exit(-1);
    }

    // "stretch" file
    int sz = _koviz_file_size();
    if ( lseek(kovizFD,sz-1,SEEK_SET) == -1 ) {
        close(kovizFD);
        fprintf(stderr,"koviz [error]: lseek failed!!! Aborting!!!\n");
        exit(-1);
    }
    if ( write(kovizFD,"",1) == -1 ) {  // write a single byte at end of file to size
        close(kovizFD);
        fprintf(stderr,"koviz [error]: writing to file=\"%s\" failed!!!  "
                       "Aborting!!!\n", fname);
        exit(-1);
    }

    // mmap the file
    kovizMap = (char*) mmap(0, sz, PROT_READ|PROT_WRITE, MAP_SHARED, kovizFD, 0);
    if ( kovizMap == MAP_FAILED ) {
        close(kovizFD);
        fprintf(stderr,"koviz [error]: mmap failed!!!  Aborting!!!\n");
        exit(-1);
    }
}

int _koviz_file_size()
{
    int sz = 0;
    sz = _koviz_header_size() + _koviz_data_size();
    return sz;
}

int _koviz_header_size()
{
    int sz = 0;

    sz += 10;          // Trick-07-L
    sz += sizeof(int); // nparams 128+1 for time

    // Calc size of header param===sys.exec.out.time
    sz += sizeof(int) + strlen("sys.exec.out.time");  // name
    sz += sizeof(int) + strlen("s");                  // unit
    sz += sizeof(int);                                // type (TRICK_07_DOUBLE===10)
    sz += sizeof(int);                                // type size (sizeof(double))

    // Calc size of a single header stat
    int statSize = 0;
    statSize += sizeof(int) + KOVIZ_NAME_SIZE ; // name
    statSize += sizeof(int) + strlen("us");     // unit (us for utime)
    statSize += sizeof(int) ;                   // type (TRICK_O7_LONG===7)
    statSize += sizeof(int) ;                   // type size (sizeof(long))

    // Add stats to header (in addition to sys.exec.out.time)
    sz += KOVIZ_MAX_STATS*statSize;

    return sz;
}

int _koviz_data_size()
{
    int sz = 0;

    int szRec = 0;
    szRec += sizeof(double) ;                // sys.exec.out.time
    szRec += sizeof(long)*KOVIZ_MAX_STATS ;  // long because utime
    kovizRecordSize = szRec;
   
    sz = KOVIZ_NRECS*szRec;

    return sz;
}

void _koviz_write_header(char* map, int fd)
{
    // Trick version
    map[0] = 'T';
    map[1] = 'r';
    map[2] = 'i';
    map[3] = 'c';
    map[4] = 'k';
    map[5] = '-';
    map[6] = '0';
    map[7] = '7';
    map[8] = '-';
    map[9] = 'L';
    int i = 10;

    // nparams
    int nparams = KOVIZ_MAX_STATS + 1; // 1 is for time
    int* nparams_addr = (int*) &map[i];
    *nparams_addr = nparams;
    i += sizeof(int);

    // sys.exec.out.time
    int* systime_name_size = (int*) &map[i];
    *systime_name_size = strlen("sys.exec.out.time");
    i += sizeof(int);

    memcpy(&map[i],"sys.exec.out.time", strlen("sys.exec.out.time"));
    i += strlen("sys.exec.out.time");

    int* s_unit_size = (int*) &map[i];
    *s_unit_size = strlen("s");
    i += sizeof(int);

    map[i] = 's';
    ++i;

    int* systime_type = (int*) &map[i];
    *systime_type = 10; // type (TRICK_07_DOUBLE===10)
    i += sizeof(int);

    int* systime_type_size = (int*) &map[i];
    *systime_type_size = sizeof(double); // time type size (double)
    i += sizeof(int);
    
    // Stats (empty param names are filled in koviz_start())
    int j = 0;
    for ( j = 0; j < KOVIZ_MAX_STATS; ++j ) {
        int* name_size = (int*) &map[i];
        *name_size = KOVIZ_NAME_SIZE;
        i += sizeof(int);

        memset(&map[i],0,KOVIZ_NAME_SIZE);  // zero it out - name added later
        i += KOVIZ_NAME_SIZE;
    
        int* unit_size = (int*) &map[i];
        *unit_size = strlen("us");
        i += sizeof(int);

        map[i+0] = 'u'; 
        map[i+1] = 's';
        i += 2;
    
        int* ptype = (int*) &map[i];
        *ptype = 7; // type (TRICK_07_LONG===7)
        i += sizeof(int);
    
        int* type_size = (int*) &map[i];
        *type_size = sizeof(long); // utime elapsed is a long
        i += sizeof(int);
    }
}
