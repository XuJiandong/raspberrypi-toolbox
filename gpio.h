#ifndef _INC_PJ_GPIO_H
#define _INC_PJ_GPIO_H

// note: gpio.h and gpio.c is original from
// https://github.com/Pieter-Jan/PJ_RPI
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <assert.h>
#include <sched.h>		// To set the priority on linux
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
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
int map_peripheral(struct bcm2835_peripheral *p);
void unmap_peripheral(struct bcm2835_peripheral *p);

// Priority
int set_program_priority(int level);

#endif
