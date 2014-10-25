

// modified from 
// https://android.googlesource.com/kernel/msm.git/+/eaf36994a3992b8f918c18e4f7411e8b2320a35f/drivers/input/misc/bmp180.c
#include "bmp180.h"
#include "main.h"
// aptitude install i2c-tools libi2c-dev 
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


#define be16_to_cpu(x) be16toh(x)
#define be32_to_cpu(x) be32toh(x)

#define BMP180_CHIP_ID                  0x55
#define BMP180_CALIBRATION_DATA_START   0xAA
#define BMP180_CALIBRATION_DATA_LENGTH  11   /* 16 bit values */
#define BMP180_CHIP_ID_REG              0xD0
#define BMP180_VERSION_REG              0xD1
#define BMP180_CTRL_REG                 0xF4
#define BMP180_TEMP_MEASUREMENT         0x2E
#define BMP180_PRESSURE_MEASUREMENT     0x34
#define BMP180_CONVERSION_REGISTER_MSB  0xF6
#define BMP180_CONVERSION_REGISTER_LSB  0xF7
#define BMP180_CONVERSION_REGISTER_XLSB 0xF8
#define BMP180_TEMP_CONVERSION_TIME     5
#define BMP180_VENDORID                 0x0001

typedef char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;

struct bmp180_calibration_data {
	s16 AC1, AC2, AC3;
	u16 AC4, AC5, AC6;
	s16 B1, B2;
	s16 MB, MC, MD;
};

struct bmp180_data {
	int client;
	struct bmp180_calibration_data calibration;
	u32 raw_temperature;
	u32 raw_pressure;
	unsigned char oversampling_setting;
	s32 b6; /* calculated temperature correction coefficient */
	struct input_dev *ip_dev;
	unsigned long delay_jiffies;
	struct bmp180_platform_data *pdata;
};

static struct bmp180_data g_data = {0};

static inline s32 bmp180_read_calibration_data(struct bmp180_data* data)
{
    u16 tmp[BMP180_CALIBRATION_DATA_LENGTH];
    struct bmp180_calibration_data *cali = &(data->calibration);
    s32 status = i2c_smbus_read_i2c_block_data(data->client,
                BMP180_CALIBRATION_DATA_START,
                sizeof(tmp),
                (u8 *)tmp);
    if (status < 0)
        return status;

    if (status != sizeof(tmp))
        return -1;

    cali->AC1 = be16_to_cpu(tmp[0]);
    cali->AC2 = be16_to_cpu(tmp[1]);
    cali->AC3 = be16_to_cpu(tmp[2]);
    cali->AC4 = be16_to_cpu(tmp[3]);
    cali->AC5 = be16_to_cpu(tmp[4]);
    cali->AC6 = be16_to_cpu(tmp[5]);
    cali->B1 = be16_to_cpu(tmp[6]);
    cali->B2 = be16_to_cpu(tmp[7]);
    cali->MB = be16_to_cpu(tmp[8]);
    cali->MC = be16_to_cpu(tmp[9]);
    cali->MD = be16_to_cpu(tmp[10]);
    return 0;
}


static inline s32 bmp180_update_raw_temperature(struct bmp180_data *data)
{
    u16 tmp;
    s32 status;

    status = i2c_smbus_write_byte_data(data->client, BMP180_CTRL_REG,
                        BMP180_TEMP_MEASUREMENT);
    if (status != 0) {
        fprintf(stderr, "Error while requesting temperature measurement.\n");
        goto exit;
    }
    usleep(BMP180_TEMP_CONVERSION_TIME*1000);

    status = i2c_smbus_read_i2c_block_data(data->client,
                    BMP180_CONVERSION_REGISTER_MSB,
                    sizeof(tmp), (u8 *)&tmp);
    if (status < 0)
        goto exit;
    if (status != sizeof(tmp)) {
        fprintf(stderr, "Error while reading temperature measurement result\n");
        status = -1;
        goto exit;
    }
    data->raw_temperature = be16_to_cpu(tmp);
    status = 0;   /* everything ok, return 0 */

exit:
    return status;
}

static inline s32 bmp180_get_temperature(struct bmp180_data *data,
                        int *temperature)
{
    struct bmp180_calibration_data *cali = &data->calibration;
    long x1, x2;
    int status;

    status = bmp180_update_raw_temperature(data);
    if (status != 0)
        goto exit;

    x1 = ((data->raw_temperature - cali->AC6) * cali->AC5) >> 15;
    x2 = (cali->MC << 11) / (x1 + cali->MD);
    data->b6 = x1 + x2 - 4000;
    /* if NULL just update b6. Used for pressure only measurements */
    if (temperature != NULL)
        *temperature = (x1+x2+8) >> 4;

exit:
    return status;
}

static inline s32 bmp180_update_raw_pressure(struct bmp180_data *data)
{
    u32 tmp = 0;
    s32 status;

    status = i2c_smbus_write_byte_data(data->client, BMP180_CTRL_REG,
                    BMP180_PRESSURE_MEASUREMENT +
                    (data->oversampling_setting<<6));
    if (status != 0) {
        fprintf(stderr, "Error while requesting pressure measurement.\n");
        goto exit;
    }

    /* wait for the end of conversion */
    usleep((2+(3 << data->oversampling_setting))*1000);

    /* copy data into a u32 (4 bytes), but skip the first byte. */
    status = i2c_smbus_read_i2c_block_data(data->client,
                    BMP180_CONVERSION_REGISTER_MSB,
                    3, ((u8 *)&tmp)+1);

    if (status < 0)
        goto exit;
    if (status != 3) {
        fprintf(stderr, "Error while reading pressure measurement results\n");
        status = -1;
        goto exit;
    }
    data->raw_pressure = be32_to_cpu((tmp));
    data->raw_pressure >>= (8-data->oversampling_setting);
    status = 0;   /* everything ok, return 0 */

exit:
    return status;
}


/*
 * This function starts the pressure measurement and returns the value
 * in millibar. Since the pressure depends on the ambient temperature,
 * a temperature measurement is executed if the last known value is older
 * than one second.
 */
static inline s32 bmp180_get_pressure(struct bmp180_data *data, int *pressure)
{
    struct bmp180_calibration_data *cali = &data->calibration;
    s32 x1, x2, x3, b3;
    u32 b4, b7;
    s32 p;
    int status;

    status = bmp180_update_raw_pressure(data);
    if (status != 0)
        goto exit;

    x1 = (data->b6 * data->b6) >> 12;
    x1 *= cali->B2;
    x1 >>= 11;

    x2 = cali->AC2 * data->b6;
    x2 >>= 11;

    x3 = x1 + x2;

    b3 = (((((s32)cali->AC1) * 4 + x3) << data->oversampling_setting) + 2);
    b3 >>= 2;

    x1 = (cali->AC3 * data->b6) >> 13;
    x2 = (cali->B1 * ((data->b6 * data->b6) >> 12)) >> 16;
    x3 = (x1 + x2 + 2) >> 2;
    b4 = (cali->AC4 * (u32)(x3 + 32768)) >> 15;

    b7 = ((u32)data->raw_pressure - b3) *
        (50000 >> data->oversampling_setting);
    p = ((b7 < 0x80000000) ? ((b7 << 1) / b4) : ((b7 / b4) * 2));

    x1 = p >> 8;
    x1 *= x1;
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
    p += (x1 + x2 + 3791) >> 4;

    *pressure = p;

exit:
    return status;
}

void bmp180_init(void) {
    int n = i2c_get_file();
    if (n == 0) {
        // TODO:
        init_i2c(1);
    }
    // TODO:
    // get by sudo i2cdetect -y 1
    i2c_set_addr(0x77);
    n = i2c_get_file();
    unsigned char version = i2c_smbus_read_byte_data(n, BMP180_CHIP_ID_REG);
    if (version != BMP180_CHIP_ID) {
        fprintf(stderr, "BMP180: wanted chip id 0x%X, "
                "found chip id 0x%X on client i2c addr 0x%X\n",
                BMP180_CHIP_ID, version, BMP180_CHIP_ID_REG);
        exit(-1);
    }
    g_data.oversampling_setting = BMP180_STANDARD;
    g_data.client = n;
    if (bmp180_read_calibration_data(&g_data) != 0) {
        fprintf(stderr, "error, bmp180_init() failed\n");
    }
}


int bmp180_get_t(void) {
    int t = -1000;
    int s = bmp180_get_temperature(&g_data, &t);
    if (s != 0) {
        fprintf(stderr, "failed to get temperature\n");
    }
    return t;
}

int bmp180_get_p(void) {
    int p = -1000;
    int s = bmp180_get_pressure(&g_data, &p);
    if (s != 0) {
        fprintf(stderr, "failed to get pressure\n");
    }
    return p;
}

int bmp180_main(int argc, const char* argv[]) {
    bmp180_init();
    double t = bmp180_get_t();
    printf("get temp = %.1f, pressure = %d\n", t/10, bmp180_get_p());
    return 0;
}
