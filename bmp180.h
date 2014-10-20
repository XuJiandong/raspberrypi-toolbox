#ifndef LINUX_BMP180_MODULE_H
#define LINUX_BMP180_MODULE_H
#include <stdbool.h>
#define BMP180_CLIENT_NAME           "bmp180"
#define BMP180_ULTRA_HIGH_RESOLUTION (3)
#define BMP180_HIGH_RESOLUTION       (2)
#define BMP180_STANDARD              (1)
#define BMP180_ULTRA_LOW_POWER       (0)

int bmp180_main(int argc, const char* argv[]);

extern void bmp180_init(void);
// temperature
extern int bmp180_get_t(void);
// pressure
extern int bmp180_get_p(void);

#endif /* LINUX_BMP180_MODULE_H */
