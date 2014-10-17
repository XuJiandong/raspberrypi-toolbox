#include "gpio.h"
#include "main.h"

int usage() {
    printf("rpi-toolbox options ...\n");
    printf("options = ud\n");
}

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        return usage();
    }
    if (strcmp(argv[1], "ud") == 0) {
        return ud_main(argc, argv);
    }
    return 0;
}



void init_gpio(void) {
    if(map_peripheral(&gpio) == -1) {
        printf("Failed to map the physical GPIO registers into the virtual memory space.\n");
        exit(-1);
    }
}

void clean_gpio(void) {
    unmap_peripheral(&gpio);
}
