#ifndef __LCD_1602_H__
#define __LCD_1602_H__

// http://arduino-info.wikispaces.com/LCD-Blue-I2C
// version 1

struct lcd1602 {
    int addr;
};

void lcd_init(struct lcd1602*);
void lcd_display(struct lcd1602*, const char* str, int line);
void lcd_clear(struct lcd1602*);

#endif // __LCD_1602_H__
