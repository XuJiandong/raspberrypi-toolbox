#include "gpio.h"
#include "main.h"
#include <linux/i2c-dev.h>

int usage() {
    printf("rpi-toolbox options ...\n");
    printf("options = util|ud|buzzer\n");
    return -1;
}

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        return usage();
    }
    if (strcmp(argv[1], "ud") == 0) {
        return ud_main(argc, argv);
    } else if (strcmp(argv[1], "buzzer") == 0) {
        return buzzer_main(argc, argv);
    } else if (strcmp(argv[1], "util") == 0) {
        return util_main(argc, argv);
    } else {
        return usage();
    }
    return 0;
}



void init_gpio(void) {
    if(map_peripheral(&g_gpio) == -1) {
        printf("Failed to map the physical GPIO registers into the virtual memory space.\n");
        exit(-1);
    }
}

void clean_gpio(void) {
    unmap_peripheral(&g_gpio);
}

static int s_file = 0;

void init_i2c(int n) {
  char fn[32] = {0};
  snprintf(fn, sizeof(fn)-1, "/dev/i2c-%d", n);
  s_file = open(fn, O_RDWR);
  if (s_file < 0) {
      fprintf(stderr, "can't open %s, not root or number number?\n", fn);
      exit(1);
  }
}

void i2c_set_addr(int addr) {
    if (ioctl(s_file, I2C_SLAVE, addr) < 0) {
        fprintf(stderr, "invalid address?\n");
        exit(1);
    }
}

int i2c_get_file(void) {
    return s_file;
}

void clean_i2c(void) {
    close(s_file);
}
