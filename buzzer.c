#include "main.h"
#include "gpio.h"

//
// run
/*
#!/bin/bash
p=500
./rpi-toolbox buzzer <<EOF
262 ${p}
262 ${p}
294 ${p}
262 ${p}
349 ${p}
330 ${p}
262 ${p}
262 ${p}
294 ${p}
262 ${p}
392 ${p}
349 ${p}
262 ${p}
262 ${p}
440 ${p}
349 ${p}
330 ${p}
294 ${p}
494 ${p}
494 ${p}
440 ${p}
349 ${p}
392 ${p}
349 ${p}
EOF
*/
 
// pin to operate on
static int s_pin = 4;
 
static int64_t s_high = 0;
inline void high(void) {
    s_high++;
    GPIO_WRITE(s_pin, 1);
}
 
static int64_t s_low = 0;
inline void low(void) {
    s_low++;
    GPIO_WRITE(s_pin, 0);
}
 
static int64_t s_count = 0;
// fre: in HZ
// elapsed: in ms
void playSound(int fre, int elapsed) {
    int64_t period = 0;
    int64_t e = 0;
    int64_t d = elapsed*1000000;
 
    s_count++;
    period = 1000000000L/fre;
    for (;;) {
        low();
        delayn(period/2);
        high();
        delayn(period/2);
        e += period;
        if (e > d) {
            break;
        }
    }
}
 
int buzzer_main(int argc, const char* argv[]) {
    if(map_peripheral(&g_gpio) == -1) {
        printf("Failed to map the physical GPIO registers into the virtual memory space.\n");
        return -1;
    }
    s_pin = 4;
    INP_GPIO(s_pin);
    OUT_GPIO(s_pin);
 
    size_t len = 64;
    char* line = (char*)malloc(len);
    int fre = 0;
    int elapsed = 0;
 
    while (getline(&line, &len, stdin) != -1) {
        int r = sscanf(line, "%d %d\n", &fre, &elapsed);
        if (r == 2) {
            if (fre < 20) {
                // no sound
                delaym(elapsed);
            } else {
                playSound(fre, elapsed);
            }
        } else {
            fprintf(stderr, "skip, invalid line: %s", line);
        }
    }
 
    printf("sound count = %lld, high = %lld, low = %lld\n", s_count, s_high, s_low);
    free(line);
    return 0;
}
