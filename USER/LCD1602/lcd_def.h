#ifndef __LCD_DEF_H
#define __LCD_DEF_H

#include "gpio.h"

#define RS PG(14)
#define RW PG(12)
#define EN PG(10)

#define D0 PG(15)
#define D1 PG(13)
#define D2 PG(11)
#define D3 PG(9)
#define D4 PD(6)
#define D5 PD(4)
#define D6 PD(2)
#define D7 PD(0)

#define SetBit(port, val) set_bit(port, val)
#define ReadBit(port) read_bit(port)
#define AntiBit(port) set_anti_bit(port)
#define SetOut(port) set_ppout(port)
#define SetIn(port) set_din(port)

// commands
#define LCD_CMD_CLEAR_DISPLAY 0x01
#define LCD_CMD_RETURN_HOME 0x02

#define LCD_CMD_ENTRY_MODE_SET 0x04
#define LCD_CMD_DISPLAY_CONTROL 0x08
#define LCD_CMD_CURSOR_SHIFT 0x10
#define LCD_CMD_FUNCTION_SET 0x20

#define LCD_CMD_SET_CGRAM_ADDR 0x40
#define LCD_CMD_SET_DDRAM_ADDR 0x80

// flags for display entry mode
#define LCD_EMS_ENTRY_RIGHT 0x00
#define LCD_EMS_ENTRY_LEFT 0x02
#define LCD_EMS_ENTRY_SHIFT_INCREMENT 0x01
#define LCD_EMS_ENTRY_SHIFT_DECREMENT 0x00

// flags for display on/off control
#define LCD_DC_DISPLAY_ON 0x04
#define LCD_DC_DISPLAY_OFF 0x00
#define LCD_DC_CURSOR_ON 0x02
#define LCD_DC_CURSOR_OFF 0x00
#define LCD_DC_BLINK_ON 0x01
#define LCD_DC_BLINK_OFF 0x00

// flags for display/cursor shift
#define LCD_CS_DISPLAY_MOVE 0x08
#define LCD_CS_CURSOR_MOVE 0x00
#define LCD_CS_MOVE_RIGHT 0x04
#define LCD_CS_MOVE_LEFT 0x00

// flags for function set
#define LCD_FS_8BIT_MODE 0x10
#define LCD_FS_4BIT_MODE 0x00
#define LCD_FS_2_LINE 0x08
#define LCD_FS_1_LINE 0x00
#define LCD_FS_5x10_DOTS 0x04
#define LCD_FS_5x8_DOTS 0x00

void lcd_init(u8 is_four_bit, u8 one);

u8 read_data_LCD(void);

u8 read_status_LCD(void);

void write_data_LCD(u8 DATA);

void write_command_LCD(u8 CMD, u8 WAIT);

void display_one_char(u8 X, u8 Y, u8 DData);

void display_list_char(u8 X, u8 Y, const char *DData);

#endif
