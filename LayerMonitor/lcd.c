#include <C:\Users\Markus\Documents\GitHub\LayerMonitor\LayerMonitor\lcd.h>

void LCD_Init (void)
{	
	char tmp = Init_LCD_Lines;
	//set port direction register to output 
	LCD_Port_DDR = LCD_DataOutput + (1<<LCD_RS | 1<<LCD_RW | 1<<LCD_E | 1<<LCD_BL);
	//wait for < 40 ms
	_delay_ms(50);			
	
	//function set 1
	LCD_Port_Write = 0X03;
	
	LCD_Port_Write |= (1<<LCD_E);
	_delay_us(1);
	LCD_Port_Write &=~ (1<<LCD_E);

	//wait for < 1.6 ms
	_delay_ms(2);	
	
	//function set 2
	LCD_Port_Write = 0X03;
	
	LCD_Port_Write |= (1<<LCD_E);
	_delay_us(1);
	LCD_Port_Write &=~ (1<<LCD_E);
	
	//wait for < 26 us
	_delay_us(50);	
	
	//function set 3
	LCD_Port_Write = 0X03;
	
	LCD_Port_Write |= (1<<LCD_E);
	_delay_us(1);
	LCD_Port_Write &=~ (1<<LCD_E);
	
	_delay_us(50);

	//function set 4

	LCD_Port_Write = 0X02;
	
	LCD_Port_Write |= (1<<LCD_E);
	_delay_us(1);
	LCD_Port_Write &=~ (1<<LCD_E);
	_delay_us(50);

	


	
	//Write_LCD (0x14,0);	//Internal OSC frequency
	//Write_LCD (0x78,0);	//Contrast set
	//Write_LCD (0x5E,0);	//Power/ICON/Contrast control
	//Write_LCD (0x6A,0);	//Follower control
	
	//Write_LCD (0x0C,0);	//Display ON/OFF control
	//Write_LCD (0x01,0);	//Clear Display
	//Write_LCD (0x06,0);	//Entry Mode set

    Write_LCD(0x29,0);       //Senden des Function set V
    Write_LCD(0x14,0);       //internal OSC frequency  0001 B10F /(bias, frequency 1 when 3 lines)
    Write_LCD(0x78,0);       //contrast set 0111 3210 (C3, C2, C1, C0 only valid when OPF1/2=0)
    Write_LCD(0x55,0);       //power 0101 IB56 (icon off, Bon high, contrast C5-C6 01) 3V:55 5V:51
    Write_LCD(0x6D,0);       //follower control 0110 F210 (Fon, Rab210 voltage ratio) 
    Write_LCD(0x0C,0);       //display ON/OFF, cursor off, cursor position off
    Write_LCD(0x01,0);       //clear LCD, cursor home
	_delay_ms(2);
    Write_LCD(0x06,0);       //cursor increment, no shift
	_delay_ms(2);
	Write_LCD(0x28,0);
}

void Write_LCD (char Data,char CD)
{
LCD_Port_DDR = LCD_DataOutput + (1<<LCD_RS | 1<<LCD_RW | 1<<LCD_E | 1<<LCD_BL);
	//control or data
	if (CD == 0)
		{
		LCD_Port_Write &=~ (1<<LCD_RS); //RS = 0 control
		}
		else
		{
		LCD_Port_Write |= (1<<LCD_RS); //RS = 1 data
		}
	//write signal
	LCD_Port_Write &=~ (1<<LCD_RW);//w = low
	
	LCD_Port_Write = (LCD_Port_Write&0xF0) + ((Data&0xF0)>>4); //Write Nibbel MSB
	//LCD_Port_Write = (LCD_Port_Write&0x0F) + (Data&0xF0); //Write Nibbel MSB (upper pins)
	
	LCD_Port_Write |= (1<<LCD_E);
	_delay_us(1);
	LCD_Port_Write &=~ (1<<LCD_E);
	_delay_us(1);	
	LCD_Port_Write = (LCD_Port_Write&0xF0) + (Data&0x0F); //Write Nibbel LSB
	//LCD_Port_Write = (LCD_Port_Write&0x0F) + ((Data&0x0F)<<4); //Write Nibbel LSB (upper pins)
	
	LCD_Port_Write |= (1<<LCD_E);
	_delay_us(1);
	LCD_Port_Write &=~ (1<<LCD_E);
	_delay_us(1);	
	
	//loop_until_bit_is_clear(Read_LCD(0),BusyBit);	
	while ((Read_LCD(0)&(1<<BusyBit)) > 0) {};
	
	_delay_us(1);

	LCD_Port_DDR = LCD_DataOutput + (1<<LCD_RS | 1<<LCD_RW | 1<<LCD_E | 1<<LCD_BL);
}

char Read_LCD (char CD)
{
	char Data;
	//Set Port Direction Register to Output for LCD Databus und LCD Steuerbus
	LCD_Port_DDR = LCD_DataInput+(1<<LCD_RS | 1<<LCD_RW | 1<<LCD_E | 1<<LCD_BL);
	//Lesesignal setzen
	LCD_Port_Write |= (1<<LCD_RW); //Zum Lesen RW-Pin = High
	//Soll ins Seuer oder Datenregister geschrieben werden?
	if (CD == 0)
		{
		LCD_Port_Write &=~ (1<<LCD_RS); //RS = 0 Steuerregister
		}
		else
		{
		LCD_Port_Write |= (1<<LCD_RS); //RS = 1 Dataregister
		}
	LCD_Port_Write |= (1<<LCD_E);//Daten stehen an wenn Enable = High
	_delay_us(1);	
	
	Data = (LCD_Port_Read&0x0F)<<4; //Lesen des 1. Nibble (MSB)
	//Data = (LCD_Port_Read&0xF0); //Lesen des 1. Nibble (MSB)
	//Data = (LCD_Port_Read&0x0F); //Lesen des 1. Nibble (MSB)
	
	LCD_Port_Write &=~ (1<<LCD_E);	
	_delay_us(1);
	
	LCD_Port_Write |= (1<<LCD_E);//Daten stehen an wenn Enable = High
	_delay_us(1);

	Data += (LCD_Port_Read&0x0F); //Lesen des 2. Nibble (LSB)
	//Data += (LCD_Port_Read&0xF0); //Lesen des 2. Nibble (LSB)
	//Data += (LCD_Port_Read&0xF0 )>>4; //Lesen des 2. Nibble (LSB)
	
	LCD_Port_Write &=~ (1<<LCD_E);
	_delay_us(1);
return(Data);
}

void LCD_Print (char line,char spalte,char *Buffer,...)
{
	va_list ap;
	va_start (ap, Buffer);	
	
	int format_flag;
	char str_buffer[10];
	char str_null_buffer[10];
	char move = 0;
	char Base = 0;
	int tmp = 0;
	
	//Berechnet Adresse für die line und schreibt sie ins DD-Ram
	if (line >= Init_LCD_Lines) //wurden mehr linen angegeben als Initialisiert
		{
		line = Init_LCD_Lines - 1;
		}
	if (line == 0)
		{
		line = 0x80;
		//line = 0x00;
		}
	if (line == 1)
		{
		line = 0x90;
		//line = 0x10;
		}
	if (line == 2)
		{
		line = 0xA0;
		//line = 0x20;
		}
	if (line == 3)
		{
		line = 0xB0;
		//line = 0x30;
		}
	line += spalte;
	Write_LCD (line,0);
	
	

	while (*Buffer != 0)
		{
		if (*Buffer == '%')
			{
			*Buffer++;
			if (isdigit(*Buffer)>0)
				{
				str_null_buffer[0] = *Buffer++;
				str_null_buffer[1] = '\0';
				move = atoi(str_null_buffer);
				}
			switch (format_flag = *Buffer++)
				{
				case 'b':
					Base = 2;
					goto ConversionLoop;
				case 'c':
					//Int to char
					format_flag = va_arg(ap,int);
					Write_LCD (format_flag++,1); 
					break;
				case 'i':
					Base = 10;
					goto ConversionLoop;
				case 'o':
					Base = 8;
					goto ConversionLoop;
				case 'x':
					Base = 16;
					//****************************
					ConversionLoop:
					//****************************
					itoa(va_arg(ap,int),str_buffer,Base);
					int b=0;
					while (str_buffer[b++] != 0){};
					b--;
					if (b<move)
						{
						move -=b;
						for (tmp = 0;tmp<move;tmp++)
							{
							str_null_buffer[tmp] = '0';
							}
						//tmp ++;
						str_null_buffer[tmp] = '\0';
						strcat(str_null_buffer,str_buffer);
						strcpy(str_buffer,str_null_buffer);
						}
					LCD_print_str (str_buffer);
					move =0;
					break;
				}
			
			}	
		else
			{
			Write_LCD (*Buffer++,1);
			}
		}
	va_end(ap);
}

void LCD_print_str (char *Buffer)
{
	while (*Buffer != 0)
		{
		Write_LCD (*Buffer++,1);
		};
}

void LCD_Clear (void)
{
	Write_LCD (1,0); //Clear Display
	//Write_LCD (0x80,0);	//Set DD-Ram Adresse = 0
	
	//Write_LCD (0x20,1); //Clear Display
	Write_LCD (0x00,0);	//Set DD-Ram Adresse = 0
}

void LCD_Contrast (char contr)
{
		contr += 4;
		Write_LCD(0x29,0);
		Write_LCD(0x54|(contr/4),0);
		Write_LCD(0x70|(contr%4*4),0);
	    Write_LCD(0x28,0);
}



