
#include "main.h"
#include "gpio.h"


static int usage() {
    printf("./rpi-toolbox util read|write [port] [0|1]");
    return -1;
}

int util_main(int argc, const char* argv[]) {
    if (argc < 5) {
        return usage();
    }
    init_gpio();
    int port = atoi(argv[3]);
    int on = atoi(argv[4]);

    if (strcmp(argv[2], "write") == 0) {
        OUT_GPIO(port);
        printf("enabled port for writing %d\n", port);
        if (on) {
            GPIO_WRITE(port, 1);
            printf("turn on port %d\n", port);
        } else {
            GPIO_WRITE(port, 0);
            printf("turn off port %d\n", port);
        }
    } else {
        INP_GPIO(port);
        printf("enabled port for reading %d\n", port);
        int on = GPIO_READ(port);
        printf("port %d status %d", port, on ? 1 : 0);
    }
    clean_gpio();
    return 0;
}

