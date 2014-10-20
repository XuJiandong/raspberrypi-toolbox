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
#include <sys/mman.h>
#include <sys/types.h>

// some code is from 
// https://github.com/Pieter-Jan/PJ_RPI
#define BCM2708_PERI_BASE       0x20000000
#define GPIO_BASE               (BCM2708_PERI_BASE + 0x200000)	// GPIO controller 
#define BLOCK_SIZE             (4*1024)

// IO Acces
struct bcm2835_peripheral {
    unsigned long addr_p;
    int mem_fd;
    void *map;
    volatile unsigned int *addr;
};

extern struct bcm2835_peripheral g_gpio;

#define INP_GPIO(g) 	*(g_gpio.addr + ((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) 	INP_GPIO(g); *(g_gpio.addr + ((g)/10)) |=  (1<<(((g)%10)*3))
#define GPIO_READ(g) 	(*(g_gpio.addr + 13) & (1<<(g)))
static inline void GPIO_WRITE(g, n) {
    if (n) {
        (*(g_gpio.addr+7))  = (1 << g);
    } else {
        (*(g_gpio.addr+10)) = (1 << g);
    }
}

// Function prototypes
extern int map_peripheral(struct bcm2835_peripheral *p);
extern void unmap_peripheral(struct bcm2835_peripheral *p);
// Priority
extern int set_program_priority(int level);

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
extern int64_t sense_range(int trig, int echo, int timeout);

// use sleep for second
#endif // __MAIN_H__
