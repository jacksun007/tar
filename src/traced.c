/* 
 * traced.c
 * 
 * Adds tracing to open and read
 *
 * Copyright 2021 University of Toronto
 *
 * Written by Kuei Sun, on 2021-12-06.  
*/

#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdint.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

/* in misc.c */
size_t blocking_read (int fd, void *buf, size_t count);

static uint64_t open_ts = 0, read_ts = 0, stat_ts = 0;
static uint64_t nr_open = 0, nr_stat = 0;

#define ts_seconds(TS) ((TS) / (uint64_t)1000000000L)
#define ts_nanosecs(TS) ((TS) % (uint64_t)1000000000L)

inline uint64_t as_nanoseconds(struct timespec* ts) {
    return ts->tv_sec * (uint64_t)1000000000L + ts->tv_nsec;
}

void print_traced(void)
{
    printf("open %lu.%09lu\n", ts_seconds(open_ts), ts_nanosecs(open_ts));
    printf("read %lu.%09lu\n", ts_seconds(read_ts), ts_nanosecs(read_ts));
    printf("stat %lu.%09lu\n", ts_seconds(stat_ts), ts_nanosecs(stat_ts));
    printf("nr_files %lu\n", nr_open);
    printf("nr_fstat %lu\n", nr_stat);   
}

int openat_traced(int dirfd, const char *pathname, int flags, ...)
{
    int err;
    int ret;
    mode_t mode = 0;
    uint64_t diff;
    struct timespec start, stop;
    
    if (flags & O_CREAT) {
        va_list vl;
        va_start(vl, flags);
        mode = va_arg(vl, mode_t);
        va_end(vl);
    }
    
    err = clock_gettime(CLOCK_REALTIME, &start);
    assert(err == 0);
    
    ret = openat(dirfd, pathname, flags, mode);
    
    err = clock_gettime(CLOCK_REALTIME, &stop);
    assert(err == 0);
    
    diff = as_nanoseconds(&stop) - as_nanoseconds(&start);
    open_ts += diff;
    nr_open += 1;
    
    //printf("open %s %lu.%09lu\n", pathname, ts_seconds(diff), ts_nanosecs(diff));
    
    return ret;
}

int fstat_traced(int fd, struct stat * buf)
{
    int err;
    int ret;
    mode_t mode = 0;
    uint64_t diff;
    struct timespec start, stop;

    err = clock_gettime(CLOCK_REALTIME, &start);
    assert(err == 0);
    
    ret = fstat(fd, buf);
    
    err = clock_gettime(CLOCK_REALTIME, &stop);
    assert(err == 0);
    
    diff = as_nanoseconds(&stop) - as_nanoseconds(&start);
    stat_ts += diff;
    nr_stat += 1;
    
    // printf("stat %lu %lu.%09lu\n", buf->st_ino, ts_seconds(diff), ts_nanosecs(diff));
    
    return ret;    
}

// TODO: this is using safe_read, which sometimes does multiple read calls
int read_traced(const char * pathname, int fd, void *buf, size_t count)
{
    int ret;
    int err;
    uint64_t diff;
    struct timespec start, stop;
    
    err = clock_gettime(CLOCK_REALTIME, &start);
    assert(err == 0);
    
    ret = blocking_read(fd, buf, count);
    
    err = clock_gettime(CLOCK_REALTIME, &stop);
    assert(err == 0);
    
    diff = as_nanoseconds(&stop) - as_nanoseconds(&start);
    read_ts += diff;
    
    //printf("read %s %lu.%09lu\n", pathname, ts_seconds(diff), 
    //       ts_nanosecs(diff));
    
    return ret;
}


