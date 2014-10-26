#include "main.h"
#include "lcd1602.h"

#define WAIT_EXECUTION() usleep(37)


#define ADDRESS  0x27

#define LCD_CLEARDISPLAY  0x01
#define LCD_RETURNHOME  0x02
#define LCD_ENTRYMODESET  0x04
#define LCD_DISPLAYCONTROL  0x08
#define LCD_CURSORSHIFT  0x10
#define LCD_FUNCTIONSET  0x20
#define LCD_SETCGRAMADDR  0x40
#define LCD_SETDDRAMADDR  0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT  0x00
#define LCD_ENTRYLEFT  0x02
#define LCD_ENTRYSHIFTINCREMENT  0x01
#define LCD_ENTRYSHIFTDECREMENT  0x00

// flags for display on/off control
#define LCD_DISPLAYON  0x04
#define LCD_DISPLAYOFF  0x00
#define LCD_CURSORON  0x02
#define LCD_CURSOROFF  0x00
#define LCD_BLINKON  0x01
#define LCD_BLINKOFF  0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE  0x08
#define LCD_CURSORMOVE  0x00
#define LCD_MOVERIGHT  0x04
#define LCD_MOVELEFT  0x00

// flags for function set
#define LCD_8BITMODE  0x10
#define LCD_4BITMODE  0x00
#define LCD_2LINE  0x08
#define LCD_1LINE  0x00
#define LCD_5x10DOTS  0x04
#define LCD_5x8DOTS  0x00

// flags for backlight control
#define LCD_BACKLIGHT  0x08 // NA
#define LCD_NOBACKLIGHT  0x00

#define EN 4 // Enable bit
#define RW 2 // Read/Write bit
#define RS 1 // Register select bit

struct lcd1602 {
    int addr;
};
static struct lcd1602 g_lcd;

inline static void WRITE(uint8_t data) {
    int file = i2c_get_file();
    i2c_smbus_write_byte(file, data);
#ifdef DEBUG_LCD
    printf("writing %d\n", data);
#endif
    WAIT_EXECUTION();
}

static void write_four_bits(uint8_t data) {
    WRITE(data);
    WRITE(data | EN);
    WAIT_EXECUTION(); // actuall less than this
    WRITE(data & ~EN);
}

void lcd_write_cmd(uint8_t d) {
    i2c_set_addr(g_lcd.addr);
    write_four_bits(d & 0xF0);
    write_four_bits((d << 4) & 0xF0);
}

void lcd_write_data(uint8_t cmd) {
    i2c_set_addr(g_lcd.addr);
    write_four_bits(RS | (cmd & 0xF0));
    write_four_bits(RS | ((cmd << 4) & 0xF0));
}

void lcd_init(void) {
    g_lcd.addr = ADDRESS;
    init_i2c(1);
    i2c_set_addr(g_lcd.addr);
    lcd_write_cmd(0x03);
    lcd_write_cmd(0x03);
    lcd_write_cmd(0x03);
    lcd_write_cmd(0x02);

    lcd_write_cmd(LCD_FUNCTIONSET | LCD_2LINE | LCD_5x8DOTS | LCD_4BITMODE);
    lcd_write_cmd(LCD_DISPLAYCONTROL | LCD_DISPLAYON);
    lcd_write_cmd(LCD_CLEARDISPLAY);
    usleep(1520);
    lcd_write_cmd(LCD_ENTRYMODESET | LCD_ENTRYLEFT);
}

void lcd_display(const char* str, int line) {
    i2c_set_addr(g_lcd.addr);
    if (line == 1) {
        lcd_write_cmd(0x80);
    } else if (line == 2) {
        lcd_write_cmd(0xC0);
    } else if (line == 3) {
        lcd_write_cmd(0x94);
    } else if (line == 4) {
        lcd_write_cmd(0xD4);
    }
    int len = strlen(str);
    int i = 0;
    for (i = 0; i < len; i++ ) {
        lcd_write_data(str[i]);
    }
}

void lcd_clear(void) {
    i2c_set_addr(g_lcd.addr);
    lcd_write_cmd(LCD_CLEARDISPLAY);
    usleep(1520); // 1.52 ms
    lcd_write_cmd(LCD_RETURNHOME);
    usleep(1520); // 1.52 ms
}

int lcd1602_main(int argc, const char* argv[]) {
    printf("lcd1602_main()\n");
    lcd_init();
    lcd_clear();
    lcd_display("hello 1", 1);
    lcd_display("hello 2", 2);
    lcd_display("hello 2", 3);
    lcd_display("hello 2", 4);
    return 0;
}

