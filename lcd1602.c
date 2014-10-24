#include "main.h"
#include "lcd1602.h"

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
#define LCD_BACKLIGHT  0x08
#define LCD_NOBACKLIGHT  0x00

#define EN 0b00000100 // Enable bit
#define RW 0b00000010 // Read/Write bit
#define RS 0b00000001 // Register select bit


inline static void WRITE(uint8_t data) {
    int file = i2c_get_file();
    i2c_smbus_write_byte(file, data);
    usleep(100);
}

static void lcd_write_four_bits(uint8_t data) {
    WRITE(data | LCD_BACKLIGHT);
    WRITE(data | EN | LCD_BACKLIGHT);
    usleep(1000*500);
    WRITE(((data & ~EN) | LCD_BACKLIGHT));
    usleep(1000*100);
}

static void lcd_write(int d) {
    lcd_write_four_bits(d & 0xF);
    lcd_write_four_bits((d << 4) & 0xF0);
}

static void lcd_write_cmd(int cmd, int mode) {
    lcd_write_four_bits(mode | (cmd & 0xF0));
    lcd_write_four_bits(mode | ((cmd << 4) & 0xF0));
}

void lcd_init(struct lcd1602* lcd) {
    lcd->addr = ADDRESS;
    init_i2c(1);
    i2c_set_addr(lcd->addr);

    lcd_write(0x03);
    lcd_write(0x03);
    lcd_write(0x03);
    lcd_write(0x02);

    lcd_write(LCD_FUNCTIONSET | LCD_2LINE | LCD_5x8DOTS | LCD_4BITMODE);
    lcd_write(LCD_DISPLAYCONTROL | LCD_DISPLAYON);
    lcd_write(LCD_CLEARDISPLAY);
    lcd_write(LCD_ENTRYMODESET | LCD_ENTRYLEFT);
    usleep(1000*200);
}

void lcd_display(struct lcd1602* lcd, const char* str, int line) {
    i2c_set_addr(lcd->addr);
     if (line == 1) {
         lcd_write(0x80);
     } else if (line == 2) {
         lcd_write(0xC0);
     } else if (line == 3) {
         lcd_write(0x94);
     } else if (line == 4) {
         lcd_write(0xD4);
     }
     int len = strlen(str);
     int i = 0;
     for (i = 0; i < len; i++ ) {
         lcd_write_cmd(str[i], RS);
     }

}

void lcd_clear(struct lcd1602* lcd) {
    i2c_set_addr(lcd->addr);
    lcd_write(LCD_CLEARDISPLAY);
    lcd_write(LCD_RETURNHOME);
}

static struct lcd1602 g_lcd;

int lcd1602_main(int argc, const char* argv[]) {
    lcd_init(&g_lcd);
    lcd_clear(&g_lcd);

    lcd_display(&g_lcd, "hello, world", 1);
    lcd_display(&g_lcd, "hello, world", 2);
    lcd_display(&g_lcd, "hello, world", 3);
    lcd_display(&g_lcd, "hello, world", 4);
}
