/*

*/

#ifndef _LCD_H_
#define _LCD_H_

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>

#include <stdio.h>

//Prototypes
extern void Write_LCD (char,char);
extern char Read_LCD (char);
extern void LCD_Init (void);
extern void LCD_Clear (void);
extern void LCD_Print (char,char,char *Buffer,...);
extern void LCD_print_str (char *Buffer);
extern void LCD_Contrast (char);

//LCD_D0 - LCD_D3 connected to Vcc
//4bit mode LCD_D4-->PORTx.0 ........ LCD_D7-->PORTx.3
//LCD_RS --> PORTx.5 | LCD_RW --> PORTx.6 | LCD_E --> PORTx.7 | PORTx.4-->NotConnect

#define Init_LCD_Lines			2

#define LCD_Port_DDR			DDRD	//LCD Port
#define LCD_Port_Write			PORTD
#define LCD_Port_Read			PIND

#define LCD_RS					7 		//Pin for RS
#define LCD_RW					6		//Pin for Read/Write
#define LCD_E					5 		//Pin for Enable
#define LCD_BL					4 		//Pin for Enable


#define LCD_DataOutput		0x0f

#define LCD_DataInput		0x00

#define BusyBit				7

#endif //_LCD_H_

