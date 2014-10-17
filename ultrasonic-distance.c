#include "main.h"
#include "gpio.h"

static int SOUND_SPEED = 343;

// use ultrasonic to detect distance
// by HC-SR04
// datasheet: http://users.ece.utexas.edu/~valvano/Datasheets/HCSR04b.pdf

// return distance in mm, -1 if timed out
// trig: trigger gpio port
// echo: echo gpio port
// timeout: timed out in ms, then return -1
// note:
// max range is 4m, that is 4/343 = 0.0117 s = 11.7 ms, and
// 11.7 * 2  = 23.4 ms, so 24 ms is the maximum timeout 
// don't need to set a very high timeout
// make it same as settle time
const static int DEFAULT_TIMEOUT = 60;

static int64_t sense_range(int trig, int echo, int timeout) {
    int64_t s = 0;

    int64_t distance = -1;
    int64_t start = 0;
    int64_t end = 0;
    // write trigger 
    // INP_GPIO(trig);
    OUT_GPIO(trig);

    // read echo
    INP_GPIO(echo);

    GPIO_CLR = 1 << trig;
    // settle
    // we suggest to use over 60ms
    // measurement cycle, in order to prevent trigger signal to the echo signal. 
    delaym(DEFAULT_TIMEOUT);

    // give a pulse, at lease 10 us
    GPIO_SET = 1 << trig;
    delayu(10);
    GPIO_CLR = 1 << trig;

    // this delay can be larger than timeout
    s = gettime() + ((int64_t)100)*1000*1000;
    while (1) {
        start = gettime();
        if (GPIO_READ(echo) == 0) {
            if (start > s) {
                printf("timed out on reading zero\n");
                distance = -1;
                goto exit;
            }
        } else {
            break;
        }
    }
    s = gettime() + ((int64_t)timeout)*1000*1000; // for timed out
    while (1) {
        end = gettime();
        if (GPIO_READ(echo) != 0) {
            if (end > s) {
                printf("timed out on reading non-zero\n");
                distance = -1;
                goto exit;
            }
        } else {
            break;
        }
    }
    // 343 m/s = 343mm/ms
    distance = (end-start)/2/1000*SOUND_SPEED/1000;
exit:
    return distance;
}

int ud_main(int argc, const char* argv[]) {
    SetProgramPriority(99);
    init_gpio();
    while (1) {
        int64_t d = sense_range(23, 24, DEFAULT_TIMEOUT);
        printf("%.1f cm (press ctrl+c to stop)\n", ((double)d)/10);
        delaym(500);
    }
    clean_gpio();
    return 0;
}

