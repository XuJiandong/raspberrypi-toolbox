local ffi = require("ffi")
local rt = ffi.load("librpi-toolbox.so")


ffi.cdef([[
extern void init_gpio(void);
extern void clean_gpio(void);
extern void enable_gpio_read(int port);
extern void enable_gpio_write(int port);
extern void write_gpio(int port, int n);
extern int read_gpio(int port);

extern void init_i2c(int n);
extern void clean_i2c(void);
extern void i2c_set_addr(int n);
extern int i2c_get_file(void);


extern void bmp180_init(void);
extern int bmp180_get_t(void);
extern int bmp180_get_p(void);
]])

function main(...)
    rt.bmp180_init()
    print(string.format("t = %f, p = %d", rt.bmp180_get_t()/10, rt.bmp180_get_p()))
end

main(...)
