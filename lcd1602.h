#ifndef __LCD_1602_H__
#define __LCD_1602_H__

// http://arduino-info.wikispaces.com/LCD-Blue-I2C
// version 1
extern void lcd_init(void);
extern void lcd_display(const char* str, int line);
extern void lcd_clear(void);
extern void lcd_write_cmd(uint8_t);
extern void lcd_write_data(uint8_t);

#endif // __LCD_1602_H__
