#ifndef _LCDSCREEN_H
#define _LCDSCREEN_H

extern void lcd_cls(void);
extern void lcd_move_cursor(void);
extern void lcd_putch(int c);
extern int  lcd_move(unsigned int y, unsigned int x);
extern void lcd_set_color(unsigned char c);
extern unsigned char lcd_get_color();
extern void lcd_init(int port);

#endif

