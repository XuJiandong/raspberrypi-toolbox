#include "main.h"

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

int64_t sense_range(int trig, int echo, int timeout) {
    int64_t s = 0;

    int64_t distance = -1;
    int64_t start = 0;
    int64_t end = 0;

    GPIO_WRITE(trig, 0);
    // settle
    // we suggest to use over 60ms
    // measurement cycle, in order to prevent trigger signal to the echo signal. 
    usleep(DEFAULT_TIMEOUT*1000);

    // give a pulse, at lease 10 us
    GPIO_WRITE(trig, 1);
    usleep(10);
    GPIO_WRITE(trig, 0);

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

static int usage(void) {
    printf("./pri-toolbox ud less|more [number in cm]\n");
    return 0;
}

int ud_main(int argc, const char* argv[]) {
    set_program_priority(99);
    init_gpio();
    int trig = 23;
    int echo = 24;
    // write trigger 
    OUT_GPIO(trig);
    // read echo
    INP_GPIO(echo);
    usleep(500*1000);
    if (strcmp(argv[2], "loop") == 0) {
        while (1) {
            int64_t d = sense_range(trig, echo, DEFAULT_TIMEOUT);
            printf("%.1f cm (press ctrl+c to stop)\n", ((double)d)/10);
            usleep(500*1000);
        }
    } else if (strcmp(argv[2], "detect") == 0) {
        int64_t d = sense_range(trig, echo, DEFAULT_TIMEOUT);
        printf("%.1f cm\n", ((double)d)/10);
    } else if (strcmp(argv[2], "less") == 0) {
        if (argc < 4) {
            printf("specify distance(in cm), e.g. ./rpi-toolbox ud less 10");
            goto exit;
        }
        int distance = atoi(argv[3])*10;
        while (1) {
            int64_t d = sense_range(trig, echo, DEFAULT_TIMEOUT);
            if (d < distance) {
                printf("detect distance(%lld cm) less than %d cm\n", d/10, distance/10);
                goto exit;
            }
            usleep(200*1000);
        }
    } else if (strcmp(argv[2], "more") == 0) {
        if (argc < 4) {
            printf("specify distance(in cm), e.g. ./rpi-toolbox ud more 10");
            goto exit;
        }
        int distance = atoi(argv[3])*10;
        while (1) {
            int64_t d = sense_range(trig, echo, DEFAULT_TIMEOUT);
            if (d > distance) {
                printf("detect distance(%lld cm) more than %d cm\n", d/10, distance/10);
                goto exit;
            }
            usleep(200*1000);
        }
    } else {
        return usage();
    }
exit:
    clean_gpio();
    return 0;
}

