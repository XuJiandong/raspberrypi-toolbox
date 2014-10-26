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

extern void lcd_init(void);
extern void lcd_display(const char* str, int line);
extern void lcd_clear(void);
extern void lcd_write_cmd(uint8_t);
extern void lcd_write_data(uint8_t);

// std
int usleep(unsigned int usec);
]])

function main(...)
    rt.bmp180_init()
    print(string.format("t = %f, p = %d", rt.bmp180_get_t()/10, rt.bmp180_get_p()))
end

function msleep(m)
    ffi.C.usleep(1000*m)
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

function LcdDisplay(str, line)
    if not str then
        print("LcdDisplay() error, specify string")
        return
    end
    if line < 0 or line > 4 then
        print("LcdDisplay() error, invalid line")
        return
    end
    if string.len(str) > 20 then
        str = string.sub(str, 1, 20)
    end
    rt.lcd_display(str, line)
end

function LcdWriteCmd(cmd)
    if type(cmd) ~= "number" then
        print("LcdWriteCmd() only accept integer")
        return 
    end
    rt.lcd_write_cmd(cmd)
end

function LcdCursor(on, blink)
    local v = 0x08 + 0x04
    if on then
        v = v + 2
    end
    if blink then
        v = v + 1
    end
    LcdWriteCmd(v)
end

function LcdOn(on)
    local v = 0x08
    if on then
        v = v + 0x04
    end
    LcdWriteCmd(v)
end

function testLcd1602(...)
    rt.lcd_init()
    print("clear screen")
    rt.lcd_clear()
    LcdDisplay("-----------", 1)
    LcdDisplay("hello,world, -------------------------------", 2)
    LcdDisplay("FBI Warning, -------------------------------", 3)
    LcdDisplay("-----------", 4)
    msleep(2000)
    LcdCursor(true, false)
    msleep(2000)
    LcdCursor(true, true)
    while true do
        msleep(1000)
        LcdOn(false)
        msleep(1000)
        LcdOn(true)
    end
end

if ... then
    testLcd1602(...)
end
