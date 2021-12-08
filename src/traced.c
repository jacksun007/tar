/* 
 * traced.c
 * 
 * Adds tracing to open and read
 *
 * Copyright 2021 University of Toronto
 *
 * Written by Kuei Sun, on 2021-12-06.  
*/

#include <fcntl.h>
#include <time.h>
#include <stdint.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

/* in misc.c */
size_t blocking_read (int fd, void *buf, size_t count);

#define ts_seconds(TS) ((TS) / (uint64_t)1000000000L)
#define ts_nanosecs(TS) ((TS) % (uint64_t)1000000000L)

inline uint64_t as_nanoseconds(struct timespec* ts) {
    return ts->tv_sec * (uint64_t)1000000000L + ts->tv_nsec;
}

int openat_traced(int dirfd, const char *pathname, int flags, ...)
{
    int err;
    int ret;
    mode_t mode = 0;
    uint64_t open_ts;
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
    
    open_ts = as_nanoseconds(&stop) - as_nanoseconds(&start);
    
    printf("open %s %lu.%09lu\n", pathname, ts_seconds(open_ts), 
           ts_nanosecs(open_ts));
    
    return ret;
}

// TODO: this is using safe_read, which sometimes does multiple read calls
int read_traced (const char * pathname, int fd, void *buf, size_t count)
{
    int ret;
    int err;
    uint64_t read_ts;
    struct timespec start, stop;
    
    err = clock_gettime(CLOCK_REALTIME, &start);
    assert(err == 0);
    
    ret = blocking_read(fd, buf, count);
    
    err = clock_gettime(CLOCK_REALTIME, &stop);
    assert(err == 0);
    
    read_ts = as_nanoseconds(&stop) - as_nanoseconds(&start);
    
    printf("read %s %lu.%09lu\n", pathname, ts_seconds(read_ts), 
           ts_nanosecs(read_ts));
    
    return ret;
}


