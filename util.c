
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
    int port = atoi(argv[4]);
    int on = atoi(argv[5]);

    if (strcmp(argv[3], "write") == 0) {
        OUT_GPIO(port);
        printf("enabled port for writing %d\n", port);
        if (on) {
            GPIO_SET = 1 << port;
            printf("turn on port %d\n", port);
        } else {
            GPIO_CLR = 1 << port;
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

