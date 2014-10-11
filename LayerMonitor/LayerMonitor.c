/*
 * LayerMonitor.c
 *
 * Created: 10/3/2014 19:46:01
 *  Author: Markus
 */ 

# define F_CPU 1000000UL

#include <util/delay.h> 
#include <avr/io.h> 
#include <avr/interrupt.h> 
#include <avr/sleep.h>       
#include <inttypes.h> 
#include <string.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <stdint.h>

#define Key_Port_DDR			DDRB	//Key Port
#define Key_Port_Write			PORTB
#define Key_Port_Read			PINB

#define	kmode					1		//key pins, Port B						
#define kreset					2
//#define kmenu					3
//#define kok					4

#define p_light					4		// Port D
#define p_buzzer				2		//Port C
#define p_batt_en				1
#define p_batt				    0

#define th						0x86	//LCD char pos
#define tm  					0x89
#define ts						0x90
#define lcd_l1					0x80
#define lcd_l2					0xC0

#define sym_micro				0x00	//symbols
#define sym_squ					0x01
#define sym_empty				0x02
#define sym_half				0x03
#define sym_full				0x04
#define sym_light				0x05
#define sym_speaker				0x06
#define sym_break				0x07

static unsigned char pattern0[8] = {17,17,17,25,22,16,16,0} ; 	//micro
static unsigned char pattern1[8] = {4,10,2,4,14,0,0,0} ; 		// squ
static unsigned char pattern2[8] = {12,30,18,18,18,18,30,0} ; 	//empty
static unsigned char pattern3[8] = {12,30,18,18,30,30,30,0} ; 	//half
static unsigned char pattern4[8] = {12,30,30,30,30,30,30,0} ; 	//full
static unsigned char pattern5[8] = {4,21,14,31,14,21,4,0} ;		//light
static unsigned char pattern6[8] = {2,6,26,26,26,6,2,0} ;		//speaker
static unsigned char pattern7[8] = {4,4,4,21,14,4,31,0} ;		//break


               
unsigned char tmp[5];
unsigned char temp;
unsigned char leadingzero;
unsigned char tchar;
unsigned char rchar;
unsigned char LCD_PWR;
unsigned char lcddata[0]; 


typedef struct{ 
unsigned char   second;      //enter the current time, date, month, and year 
unsigned char   minute; 
unsigned char   hour; 
unsigned char   date; 
unsigned char   month; 
unsigned int    year;
unsigned char   run;
         }time; 
time t;

unsigned char menu;
unsigned char timer_LCD;
unsigned char timer_light;
unsigned char timer_menu;
unsigned char timer_key;
unsigned char speaker;
unsigned char light;
unsigned char battery;
unsigned char contrast;
unsigned char cc;
unsigned char mode;

int batt;
long f_rel;
long f_zero;
long f_abs;
long l_rel;
long l_abs;
long l_zero;

int main(void)             
{ 
  DDRB=0x29;
  PORTB=0x06;

  DDRC=0xFE;
  PORTC=0x06;
   
  tchar = ':';

  menu = 0;
  timer_menu = 0;
  contrast = 2;
  timer_LCD = 10;
  timer_key = 0;
  mode = 'A';
  f_rel = 0;
  l_rel = 0;
  
  LCD_Wake();

  //LCD_Port_Write |= (1<<LCD_BL)

	PCMSK0 |= (1<<PCINT1); //  tell pin change mask to listen to pin15
   	PCMSK0 |= (1<<PCINT2); //  tell pin change mask to listen to pin16
   	//PCMSK0 |= (1<<PCINT3); //  tell pin change mask to listen to pin17
   	//PCMSK0 |= (1<<PCINT4); //  tell pin change mask to listen to pin17
   	PCICR  |= (1<<PCIE0); // enable PCINT interrupt in the general interrupt mask


    TIMSK2 = 0x00;
    
	ASSR = 0x20;						//crystal on T2 32,768kHz

    TCNT2 = 0x00; 						//counter = 0 
    
	TCCR2B = 0x05;						//divide by 128

    while(ASSR&0x1F);          			//Wait until TC2 is updated 

    TIFR2 |= ((1<<OCF2A)|(1<<OCF2B)|(1<<TOV2)); 

	TIMSK2 = 0x01;			       		//set 8-bit Timer/Counter2 Overflow Interrupt Enable 
										//TIMSK2 |= (1<<TOIE2);	//Clear the Timer/Counter2 Interrupt Flags.
										
	SPI_init();
   
   	sei();

    while(1) 
    { 
     
	    OCR2A = 0;                       // dummy access
        while((ASSR & (1<< OCR2AUB)));   // wait for finish
 
        set_sleep_mode(SLEEP_MODE_PWR_SAVE);
        sleep_mode();                   // chnage to sleep mode
 
    } 
return 0; 

} 


/*****************************************************************************
*
*	Function name : SPI_init
*
*	Returns :		None
*
*	Parameters :	None
*
*	Purpose :		Sets up the HW SPI in Master mode, Mode 3
*					Note -> does notuse SS line to control the DF CS-line.
*
******************************************************************************/
void SPI_init (void)
{
	PORTB |= (1<<PB3) | (1<<PB2) | (1<<PB1); // MSt SS not used | (1<<PB0)
	DDRB |= (1<<DDB2) | (1<<DDB1);		//Set MOSI, SCK  (MSt | (1<<DDB0) SS as outputs)

	//SPSR = (1<<SPI2X);                                      //SPI double speed settings
	SPCR = (1<<SPE) | (1<<MSTR) | (1<<CPHA) | (1<<CPOL);	//Enable SPI in Master mode, mode 3, Fosc/4
}

/*****************************************************************************
*
*	Function name : SPI_RW
*
*	Returns :		Byte read from SPI data register (any value)
*
*	Parameters :	Byte to be written to SPI data register (any value)
*
*	Purpose :		Read and writes one byte from/to SPI master
*
******************************************************************************/
unsigned char SPI_RW (unsigned char output)
{
	unsigned char input;
	
	SPDR = output;							//put byte 'output' in SPI data register
	while(!(SPSR & 0x80));					//wait for transfer complete, poll SPIF-flag
	input = SPDR;							//read value in SPI data reg.
	
	return input;							//return the byte clocked in from SPI slave
}

long get_freq ()
{
	unsigned char byteFreq;
	unsigned char outChar;
	long freq;
	
	outChar = 'A';
	byteFreq = SPI_RW (outChar);
	_delay_us(100);
	
	outChar = '3';
	byteFreq = SPI_RW (outChar);
	freq = byteFreq;
	_delay_us(100);
	
	outChar = '2';
	byteFreq = SPI_RW (outChar);
	freq = freq * 256 + byteFreq ;
	_delay_us(100);
	
	outChar = '1';
	byteFreq = SPI_RW (outChar);
	freq = freq * 256 + byteFreq ;
	_delay_us(100);
	
	outChar = '0';
	byteFreq = SPI_RW (outChar);
	freq = freq * 256 + byteFreq ;
	_delay_us(100);
	

	if ((freq < 0) |(freq > 9000000)) freq = 0;

	return freq;
}



ISR(TIMER2_OVF_vect)         //overflow interrupt vector 
{ 
	if (timer_key > 1) timer_key++;

	if (timer_key > 3)
	{
		timer_key = 1;

		if ((PINB & (1<<kmode)) == 0)         //no use 
	    { 


	    }

		if ((PINB & (1<<kreset)) == 0)         //reset thickness
	    { 
			Write_LCD (0x80,0);
			LCD_print_str ("     ");
	    }
		
/*		if (((PINB & (1<<kok)) == 0) & (menu == 0))         //sleep
	    { 
			LCD_Sleep();
		 }
*/			
		/*if (((PINB & (1<<kmenu)) == 0) & (menu == 0))         //enter menu
	    { 
			 switch (menu)
			 {
				case 0:
				++menu;
				Write_LCD (0x80,0);
				LCD_print_str   ("Kontrast:      ");
				Write_LCD (0x8F,0);
				Write_LCD (contrast+48,1);
				comp_contrast(contrast);

				break;
			}
		
		}*/
	}


	if (timer_menu != 0)
	{
		if (--timer_menu == 0)
		{
			menu = 0;
		}

	}




	if (++t.second==60)         //keep track of time, date, month, and year 
    { 
        t.second=0; 
        if (++t.minute==60) 
        { 
            t.minute=0; 
            if (++t.hour==24) 
            { 
                t.hour=0;
            }
			if (LCD_PWR != 0 & menu == 0)
			{
				batt = battery_voltage();
				battery_symbol();
				comp_contrast(contrast);
			}
			if (timer_LCD != 0)
			{
				if (--timer_LCD == 0)
				{
					LCD_Sleep();
				}

			}
 

        }
		
		if (timer_light != 0) 
		{
			if (--timer_light == 0)
			{
				light = 0;
				timer_light = 0;
				PORTD &=~ (1<<p_light); 
			}

		}

    } 
	
	if (LCD_PWR == 0x00) return;
 
	PORTC |= (1<<p_buzzer); 	

	if (mode=='A')
	{		
		unsigned char valuetext[8];

		f_abs = get_freq();
		

		//f_abs = 1234567;
		//ltoa(f_abs, valuetext, 10);

		double	f_dbl;
		f_dbl = f_abs;
		dtostrf(f_dbl,8,0,valuetext);

		Write_LCD (lcd_l2+5,0);
		LCD_print_str   (valuetext);
	}


	if (mode=='R')
	{
		unsigned char valuetext[8];
		
		f_abs = get_freq();

		//f_abs = 1234567;
		//ltoa(f_abs, valuetext, 10);

		double	f_dbl;
		f_dbl = f_abs - f_zero;
		dtostrf(f_dbl,8,0,valuetext);

		Write_LCD (lcd_l2+5,0);
		LCD_print_str   (valuetext);
	}

	if (mode=='L')
	{
		long f_sig;
		unsigned char valuetext[7];

			
		double	f_dbl;
		f_dbl = -22.3333;

		dtostrf(f_dbl,6,2,valuetext);

		Write_LCD (lcd_l2+0,0);
		LCD_print_str   (valuetext);
		
	

		//read frequency and output
	}




} 

ISR(PCINT0_vect)					//key pressed
{
	_delay_ms(80);

	if (((PINB & (1<<kmode)) == (1<<kmode)) & ((PINB & (1<<kreset)) == (1<<kreset))) 

	{
		timer_key = 0;
		return;
	}
	_delay_ms(10);

	if (((PINB & (1<<kmode)) == (1<<kmode)) & ((PINB & (1<<kreset)) == (1<<kreset))) 
 
	{
		timer_key = 0;
		return;
	}

	if (!LCD_PWR)
	{
		LCD_Wake();
		timer_LCD = 10;
		return;			
	}


	if (timer_LCD != 0)
	{
		timer_LCD = 10;

	}


	if (timer_light != 0)
	{
		timer_light = 20;

	}

	if ((PINB & (1<<kmode)) == 0)         //mode change
    { 
		switch (mode)
		{
			case 'A':
				mode = 'R';
				mode_text(mode);								
			break;
			
			case 'R':
				mode = 'L';
				mode_text(mode);								
			break;
			
			case'L':
				mode = 'A';
				mode_text(mode);								
			break;
		}
		
    }


	if ((PINB & (1<<kreset)) == 0)         //reset 
    { 
		
		f_zero = f_abs;
		l_zero = l_abs;
    }

/*	if ((PINB & (1<<kok)) == 0)         //Select
    { 
		 timer_menu = 20; 
		 switch (menu)
		 {
			case 1:
				if (++contrast==4) 
			    { 
			        contrast=1; 
			    }
				Write_LCD (0x8F,0);
				Write_LCD (contrast+48,1);
				comp_contrast(contrast);
				break;

			case 2:
				break;
			
			default:
				timer_menu = 0;
				if (light == 0)
				{
					light = 1;
					timer_light = 20;
					PORTD |= (1<<p_light);
					comp_contrast(contrast);
					
					
				}
				else
				{
					light = 0;
					timer_light = 0;
					PORTD &=~ (1<<p_light);
					comp_contrast(contrast); 
				}			

		 }

    }
*/
/*	if ((PINB & (1<<kmenu)) == 0)         //Menu
    { 
		 timer_menu = 20; 
		 switch (menu)
		 {
			case 1:
				++menu;
				Write_LCD (0x80,0);
  				LCD_print_str   ("Battery:       V");
				batt=battery_voltage();
				Write_LCD (0x8A,0);
				Write_LCD (batt/100+48,1);
				Write_LCD ('.',1);
				Write_LCD (batt%100/10+48,1);
				Write_LCD (batt%10+48,1);
				break;

			case 2:
				++menu;
				Write_LCD (0x80,0);
  				LCD_print_str   ("by stoeckli.net ");
				break;
			
			default:
				menu = 0;
				mode_text (mode);
				//LCD_Time();
		 }

    }
	*/
	timer_key = 2;


	
	
}



void LCD_build(unsigned char location, unsigned char *ptr){
      unsigned char i;
      if(location<8){
          Write_LCD(0x40 + (location*8),0);
		  for(i=0;i<8;i++)
             Write_LCD(ptr[ i ],1);
     }
}

void LCD_Sleep(void)
{
	PORTD = 0x00;
	PORTB |= 0x01;
	LCD_PWR = 0x00;
	timer_LCD = 0; 
}
 
void LCD_Time(void)
{
	Write_LCD (0x80,0);
	LCD_print_str   ("                ");

	LCD_out_h();
	LCD_out_m();

	
    battery_symbol();

}


void LCD_Wake(void)
{
	LCD_PWR = 0x01;
	PORTB &= 0xFE;
	_delay_ms(100);
	LCD_Init();
	mode_text(mode);
  	Write_LCD (0xD0,0);
  	LCD_print_str   ("                ");

	LCD_build(0,pattern0);
	LCD_build(1,pattern1);
	LCD_build(2,pattern2);
	LCD_build(3,pattern3);
	LCD_build(4,pattern4);
	LCD_build(5,pattern5);
	LCD_build(6,pattern6);
	LCD_build(7,pattern7);

	batt = battery_voltage();
    battery_symbol();
	comp_contrast(contrast);


}

void LCD_out_h(void)
{
	Write_LCD (th,0);
	Write_LCD (t.hour/10+48,1);
	Write_LCD (t.hour%10+48,1);
}

void LCD_out_m(void)
{
	Write_LCD (tm,0);
	Write_LCD (t.minute/10+48,1);
	Write_LCD (t.minute%10+48,1);
}

void LCD_out_s(void)
{
	Write_LCD (ts,0);
  	Write_LCD (t.second/10+48,1);
	Write_LCD (t.second%10+48,1);
}






int battery_voltage(void)
{
	int voltage;
	PORTC &=~ (1<<p_batt_en);
	_delay_ms(10);

   	PRR &=~ (1<<PRADC); 						//enable ADC power
   	ADCSRA = (1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2);	//clock divide by 128 
   	DIDR0 = (1<<ADC0D);							//disable digital input on ADC0 
   	ADMUX = 0xC0;//(1<<MUX0); 


      ADCSRA |= (1<<ADSC)|(1<<ADEN); 
         while((ADCSRA & (1<<ADSC))!=0) ; 
          
         voltage=ADC; 

 	ADCSRA &=~ (1<<ADEN);
    PRR |= (1<<PRADC);							//diable ADC Power
	PORTC |= (1<<p_batt_en);	
	return (voltage);
}
 
int comp_contrast(int contrast)
{
	batt = battery_voltage();
	switch (batt)
	{
		case 000 ... 259:
			cc = 9;
		break;
		case 260 ... 264:
			cc = 8;
		break;
		case 265 ... 269:
			cc = 7;
		break;
		case 270 ... 274:
			cc = 6;
		break;
		case 275 ... 279:
			cc = 5;
		break;
		case 280 ... 299:
			cc = 4;
		break;
		case 300 ... 319:
			cc = 3;
		break;
		case 320 ... 349:
			cc = 2;
		break;
		case 350 ... 369:
			cc = 1;
		break;
		case 370 ... 500:
			cc = 0;
		break;		

	}
	
	switch (contrast)
	{
		case 1:
			if (cc != 0) cc--;
		break;

		case 2:
		break;
		case 3:
			if (cc != 9) cc++;
		break;
	}
	
	LCD_Contrast (cc);	

}

void battery_symbol(void)
{
	Write_LCD (0x8F,0);
	if (batt > 260) //280 for Cr2032
	{
		Write_LCD (sym_full,1);
		return;
	}

	if (batt > 220) //265 for Cr2032
	{
		Write_LCD (sym_half,1);
		return;
	}

		Write_LCD (sym_empty,1);
}



void mode_text (char md)
{
	switch (md)
	{
		case 'A':
			Write_LCD (lcd_l1,0);
			LCD_print_str("ABS. FREQUENCY ");
			Write_LCD (lcd_l2,0);
			LCD_print_str("              Hz");
		break;
			
		case 'R':
			Write_LCD (lcd_l1,0);
			LCD_print_str("REL. FREQUENCY ");
			Write_LCD (lcd_l2,0);
			LCD_print_str("              Hz");								
		break;
			
		case'L':
			Write_LCD (lcd_l1,0);
			LCD_print_str("LAYER          ");
			Write_LCD (lcd_l2,0);
			LCD_print_str("           g/cm ");
			Write_LCD(lcd_l2+10,0);
			Write_LCD(sym_micro,1);
			Write_LCD(lcd_l2+15,0);
			Write_LCD(sym_squ,1);				
		break;
	}
}


