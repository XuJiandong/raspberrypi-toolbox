#include "main.h"
#include "bmp180.h"

struct bcm2835_peripheral g_gpio = {GPIO_BASE};

// Exposes the physical address defined in the passed structure using mmap on /dev/mem
int map_peripheral(struct bcm2835_peripheral *p) {
   // Open /dev/mem
   if ((p->mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      printf("Failed to open /dev/mem, try checking permissions.\n");
      return -1;
   }
   p->map = mmap(NULL, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, 
      p->mem_fd,  // File descriptor to physical memory virtual file '/dev/mem'
      p->addr_p   // Address in physical map that we want this memory block to expose
   );
   if (p->map == MAP_FAILED) {
        perror("mmap");
        return -1;
   }
   p->addr = (volatile unsigned int *)p->map;
   return 0;
}

void unmap_peripheral(struct bcm2835_peripheral *p) {
    munmap(p->map, BLOCK_SIZE);
    close(p->mem_fd);
}

// Priority 
int set_program_priority(int priorityLevel) {
    struct sched_param sched;
    memset (&sched, 0, sizeof(sched));
    if (priorityLevel > sched_get_priority_max (SCHED_RR))
        priorityLevel = sched_get_priority_max (SCHED_RR);
    sched.sched_priority = priorityLevel;
    return sched_setscheduler (0, SCHED_RR, &sched);
}

int usage() {
    printf("rpi-toolbox options ...\n");
    printf("options = util|ud|buzzer|bmp180\n");
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
    } else if (strcmp(argv[1], "bmp180") == 0) {
        return bmp180_main(argc, argv);
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

void enable_gpio_read(int port) {
    INP_GPIO(port);
}

void enable_gpio_write(int port) {
    OUT_GPIO(port);
}

void write_gpio(int port, int n) {
    GPIO_WRITE(port, n);
}

int read_gpio(int port) {
    return GPIO_READ(port) ? 1 : 0;
}
