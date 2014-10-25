local ffi = require("ffi")
local rt = ffi.load("librpi-toolbox.so")


ffi.cdef([[
// user defined
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
extern void i2c_write_byte(uint8_t);


extern void bmp180_init(void);
extern int bmp180_get_t(void);
extern int bmp180_get_p(void);

struct lcd1602 {
    int addr;
};

extern void lcd_init(struct lcd1602*);
extern void lcd_display(struct lcd1602*, const char* str, int line);
extern void lcd_clear(struct lcd1602*);

// std
int usleep(unsigned int usec);
]])

function main(...)
    rt.bmp180_init()
    print(string.format("t = %f, p = %d", rt.bmp180_get_t()/10, rt.bmp180_get_p()))
end

function testGpio()
    rt.init_gpio()
    rt.enable_gpio_write(18)
    rt.write_gpio(18, 1)
    ffi.C.usleep(1000*1000);
    rt.write_gpio(18, 0)
end

function testing()
    ffi.C.usleep(1000*1000)
    print("done")
end

function testI2c(...)
    local argv = {...}
    local v = 1
    if argv[1] then
        v = tonumber(argv[1])
    end
    rt.init_i2c(1)
    rt.i2c_set_addr(0x27)
    print("write " .. tostring(v) .. " to 0x27 on i2c")
    rt.i2c_write_byte(v)
end

function testLcd1602(...)
    local lcd = ffi.new("struct lcd1602[1]")
    rt.lcd_init(lcd)
    print("clear screen")
    rt.lcd_clear(lcd)
    print("display on line 1")
    rt.lcd_display(lcd, "xxxxxxxxxxxxxxxxxxxxxxxxxxxx", 1)
    print("display on line 2")
    rt.lcd_display(lcd, "hello,world 2", 2)
    print("display on line 3")
    rt.lcd_display(lcd, "hello,world 3", 3)
    print("display on line 4")
    rt.lcd_display(lcd, "hello,world 4", 4)
end

if ... then
    testLcd1602(...)
end
