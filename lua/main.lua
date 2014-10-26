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


-- LCD part
function LcdPrintln(str, line)
    if not str then
        print("LcdPrintln() error, specify string")
        return
    end
    if line < 0 or line > 4 then
        print("LcdPrintln() error, invalid line")
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

function LcdEnableShift(e)
    local v = 0x06
    if e then
        v = v + 1
    end
    LcdWriteCmd(v)
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

function LcdBlink(cycle, time)
    local elapsed = 0
    while true do
        LcdOn(true)
        msleep(cycle)
        LcdOn(false)
        msleep(500)
        elapsed = elapsed + cycle + 500
        if elapsed > time then
            break
        end
    end
    LcdOn(true)
end

function LcdOutput(...)
    rt.lcd_init()
    rt.lcd_clear()
    for i, v in ipairs({...}) do
        if i <= 4 then
            LcdPrintln(v, i)
        else
            break
        end
    end
end

function testLcd1602(...)
    rt.lcd_init()
    rt.lcd_clear()
    LcdPrintln("-----------", 1)
    LcdPrintln("hello,world, -------------------------------", 2)
    LcdPrintln("FBI Warning, -------------------------------", 3)
    LcdPrintln("-----------", 4)
    msleep(2000)
    LcdCursor(true, false)
    print("show cursor")
    msleep(2000)
    LcdCursor(true, true)
    print("cursor blink")
    
    LcdBlink(2000, 10000)

    local count = 0
    while true do
        msleep(1000)
        io.write(".")
        io.flush()
        count = count + 1
        if count >= 60 then
            io.write("\n")
            io.flush()
            count = 0
        end
    end
end

function usage()
    print("sudo bash run.sh [lcdtest|lcd]")
    return 1
end

if ... then
    local args = {...}
    if args[1] == "lcdtest" then
        testLcd1602(...)
    elseif args[1] == "lcd" then
        table.remove(args, 1)
        LcdOutput(unpack(args))
    else
        return usage()
    end
else
    return usage()
end
