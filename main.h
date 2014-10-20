#ifndef __MAIN_H__
#define __MAIN_H__
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <assert.h>
#include <sched.h>
#include <unistd.h>
#include <inttypes.h>

extern void init_gpio(void);
extern void clean_gpio(void);
// can be used by luajit ffi
extern void enable_gpio_read(int port);
extern void enable_gpio_write(int port);
extern void write_gpio(int port, int n);
extern int read_gpio(int port);

extern void init_i2c(int n);
extern void clean_i2c(void);
extern void i2c_set_addr(int n);
extern int i2c_get_file(void);


extern int ud_main(int argc, const char* argv[]);
extern int buzzer_main(int argc, const char* argv[]);
extern int util_main(int argc, const char* argv[]);

static inline void delayn(long ns) {
    struct timespec t = {0};
    t.tv_nsec = ns;
    nanosleep(&t, NULL);
}

static inline void delayu(long us) {
    struct timespec t = {0};
    t.tv_nsec = us*1000;
    nanosleep(&t, NULL);
}

extern void delaym(long ms);

static inline int64_t gettime(void) {
    struct timespec t = {0};
    clock_gettime(CLOCK_MONOTONIC, &t);
    int64_t n = t.tv_sec;
    n = n*1000*1000*1000 + t.tv_nsec;
    return n;
}

// use sleep for second
#endif // __MAIN_H__
