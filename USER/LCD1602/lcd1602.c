#include "lcd_def.h"
#include "gpio.h"
#include "threads.h"

u8 four_bit = 0;

u8 one_line = 0;

u8 start = 0;

u8 type_data_line = LCD_FS_8BIT_MODE;

PortType DataPorts[] = {
    D0,
    D1,
    D2,
    D3,
    D4,
    D5,
    D6,
    D7};

void set_data_out(void)
{
    for (int i = start; i < 8; i++)
        SetOut(DataPorts[i]);
}

void set_data_in(void)
{
    for (int i = start; i < 8; i++)
        SetIn(DataPorts[i]);
}

void set_high()
{
    for (int i = start; i < 8; i++)
        SetBit(DataPorts[i], 1);
}

void set_data(u8 DATA)
{
    for (int i = start; i < 8; i++)
        SetBit(DataPorts[i], DATA & (0x1 << i));
    SetBit(EN, 1);
    delay_us(1);
    SetBit(EN, 0);
    if (four_bit)
    {
        delay_us(1);
        for (int i = 0; i < 4; i++)
            SetBit(DataPorts[i + 4], DATA & (0x1 << i));
        SetBit(EN, 1);
        delay_us(1);
        SetBit(EN, 0);
    }
}

u8 read_data()
{
    u8 data = 0xff;
    for (int i = start; i < 8; i++)
        data &= (ReadBit(DataPorts[i]) << i);
    SetBit(EN, 0);
    if (four_bit)
    {
        delay_us(1);
        SetBit(EN, 1);
        delay_us(1);
        for (int i = 0; i < 4; i++)
            data &= (ReadBit(DataPorts[i + 4]) << i);
        SetBit(EN, 0);
    }
    return data;
}

void lcd_init(u8 is_four_bit, u8 one)
{
    if (is_four_bit)
    {
        four_bit = 1;
        start = 4;
        type_data_line = LCD_FS_4BIT_MODE;
    }
    if (one)
    {
        one_line = 1;
    }
    SetOut(RS);
    if (!one_line)
        SetOut(RW);
    SetOut(EN);
    set_data_out();
    set_data(0x0);
    if (four_bit)
    {
        // this is according to the hitachi HD44780 datasheet
        // figure 24, pg 46
        write_command_LCD(0x03, 0);
        delay_ms(5);
        write_command_LCD(0x03, 0);
        delay_ms(5);
        write_command_LCD(0x03, 0);
        delay_ms(1);
        write_command_LCD(0x02, 0);
    }
    else
    {
        // this is according to the hitachi HD44780 datasheet
        // page 45 figure 23
        write_command_LCD(LCD_CMD_FUNCTION_SET | type_data_line | LCD_FS_5x8_DOTS | LCD_FS_2_LINE, 0);
        delay_ms(5);
        write_command_LCD(LCD_CMD_FUNCTION_SET | type_data_line | LCD_FS_5x8_DOTS | LCD_FS_2_LINE, 0);
        delay_ms(1);
        write_command_LCD(LCD_CMD_FUNCTION_SET | type_data_line | LCD_FS_5x8_DOTS | LCD_FS_2_LINE, 0);
    }
    write_command_LCD(LCD_CMD_FUNCTION_SET | type_data_line | LCD_FS_5x8_DOTS | LCD_FS_2_LINE, 1);
    write_command_LCD(LCD_CMD_DISPLAY_CONTROL | LCD_DC_DISPLAY_ON | LCD_DC_CURSOR_OFF | LCD_DC_BLINK_OFF, 1);
    write_command_LCD(LCD_CMD_CLEAR_DISPLAY, 1);
    write_command_LCD(LCD_CMD_ENTRY_MODE_SET | LCD_EMS_ENTRY_LEFT | LCD_EMS_ENTRY_SHIFT_DECREMENT, 1);
    write_command_LCD(LCD_CMD_CURSOR_SHIFT | LCD_CS_CURSOR_MOVE | LCD_CS_MOVE_RIGHT, 1);
}

u8 read_data_LCD(void)
{
    if (one_line)
        return 0x0;
    SetBit(EN, 0);
    set_high();
    set_data_in();
    SetBit(RS, 1);
    SetBit(RW, 1);
    SetBit(EN, 1);
    delay_us(1);
    u8 data = read_data();
    set_data_out();
    return data;
}

u8 read_status_LCD(void)
{
    if (one_line)
    {
        delay_us(1);
        return 0x0;
    }
    SetBit(EN, 0);
    set_high();
    set_data_in();
    SetBit(RS, 0);
    SetBit(RW, 1);
    SetBit(EN, 1);
    delay_us(1);
    while (ReadBit(D7))
        ;
    u8 data = read_data();
    set_data_out();
    return data;
}

void write_data_LCD(u8 DATA)
{
    read_status_LCD();
    SetBit(EN, 0);
    SetBit(RS, 1);
    if (!one_line)
        SetBit(RW, 0);
    set_data(DATA);
}

void write_command_LCD(u8 CMD, u8 WAIT)
{
    if (WAIT)
        read_status_LCD();
    SetBit(EN, 0);
    SetBit(RS, 0);
    if (!one_line)
        SetBit(RW, 0);
    set_data(CMD);
}

uint8_t on_display = 0;

void display_one_char(u8 X, u8 Y, u8 DData)
{
		while(on_display)
			;
		on_display = 1;
    Y &= 0x1;
    X &= 0xf;
    if (Y)
        X |= 0x40;
    X |= 0x80;
    write_command_LCD(X, 0);
    write_data_LCD(DData);
		on_display = 0;
}

void display_list_char(u8 X, u8 Y, const char *DData)
{
    u8 pos = 0;
    Y &= 0x1;
    X &= 0xf;
    while (DData[pos] >= 0x20)
    {
        if (X <= 0xF)
        {
            display_one_char(X, Y, (u8)DData[pos]);
            pos++;
            X++;
        }
    }
}
