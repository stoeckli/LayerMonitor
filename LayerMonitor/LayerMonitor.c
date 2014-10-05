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

#define	k1						1		//key pins, Port B						
#define k2						2
#define kmenu					3
#define kok						4

#define p_light					4		// Port D
#define p_buzzer				2		//Port C
#define p_batt_en				1
#define p_batt				    0

/*
#define th						0x84	//LCD char pos
#define tm  					0x87					
#define ts						0x8A
#define t1h						0xC0
#define t1m						0xC2
#define t1s						0xC5
#define t2h						0xC9
#define t2m						0xCB
#define t2s						0xCE
*/

#define th						0x86	//LCD char pos
#define tm  					0x89
#define ts						0x90
#define t1h						0xD0
#define t1m						0xC0
#define t1s						0xC3
#define t1al					0x80
#define t2h						0xD0
#define t2m						0xCB
#define t2s						0xCE
#define t2al					0x8F

#define sym_flame1				0x00	//symbols
#define sym_flame2				0x01
#define sym_empty				0x02
#define sym_half				0x03
#define sym_full				0x04
#define sym_light				0x05
#define sym_speaker				0x06
#define sym_break				0x07


static unsigned char pattern0[8] = {2,4,12,30,31,31,14,0} ; 		//flame 1
static unsigned char pattern1[8] = {0,4,6,14,31,31,14,0} ; 		// flame 2
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
unsigned char   run
         }time; 
time t;
time t1;
time t2;

unsigned char menu;
unsigned char timer_LCD;
unsigned char timer_light;
unsigned char timer_menu;
unsigned char timer_key;
unsigned char speaker;
unsigned char light;
unsigned char alarm_mode_1;
unsigned char alarm_mode_2;
unsigned char alarm_time_1;
unsigned char alarm_time_2;
unsigned char battery;
unsigned char contrast;
unsigned char cc;

int batt;


int main(void)             
{ 
	 
 
  DDRB=0x01;
  PORTB=0x3E;

  DDRC=0xFE;
  PORTC=0x06;

   
  tchar = ':';
  alarm_mode_1 = 0x01;
  alarm_mode_2 = 0x02;
  alarm_time_1 = 10;
  alarm_time_2 = 10; 
  menu = 0;
  timer_menu = 0;
  contrast = 2;
  timer_LCD = 10;
  timer_key = 0;
  
  LCD_Wake();

  //LCD_Port_Write |= (1<<LCD_BL)

 

	PCMSK0 |= (1<<PCINT1); //  tell pin change mask to listen to pin15
   	PCMSK0 |= (1<<PCINT2); //  tell pin change mask to listen to pin16
   	PCMSK0 |= (1<<PCINT3); //  tell pin change mask to listen to pin17
   	PCMSK0 |= (1<<PCINT4); //  tell pin change mask to listen to pin17
   	PCICR  |= (1<<PCIE0); // enable PCINT interrupt in the general interrupt mask


    TIMSK2 = 0x00;
    
	ASSR = 0x20;						//crystal on T2 32,768kHz

    TCNT2 = 0x00; 						//counter = 0 
    
	TCCR2B = 0x05;						//divide by 128

    while(ASSR&0x1F);          			//Wait until TC2 is updated 

    TIFR2 |= ((1<<OCF2A)|(1<<OCF2B)|(1<<TOV2)); 

	TIMSK2 = 0x01;			       		//set 8-bit Timer/Counter2 Overflow Interrupt Enable 
										//TIMSK2 |= (1<<TOIE2);	//Clear the Timer/Counter2 Interrupt Flags.
   
   	sei();

    while(1) 
    { 
     
	    OCR2A = 0;                       // Dummyzugriff
        while((ASSR & (1<< OCR2AUB)));   // Warte auf das Ende des Zugriffs
 
        set_sleep_mode(SLEEP_MODE_PWR_SAVE);
        sleep_mode();                   // in den Schlafmodus wechseln
 
    } 
return 0; 

} 

ISR(TIMER2_OVF_vect)         //overflow interrupt vector 
{ 
	if (timer_key > 1) timer_key++;

	if (timer_key > 3)
	{
		timer_key = 1;

		if ((PINB & (1<<k1)) == 0)         //timer1
	    { 
			t1.second=0;
			t1.minute=0;
			t1.hour=0;
			Write_LCD (t1m,0);
			LCD_print_str ("     ");
			t1.run = (0x00);
	    }

		if ((PINB & (1<<k2)) == 0)         //timer2 
	    { 
			t2.second=0;
			t2.minute=0;
			t2.hour=0;
			Write_LCD (t2m,0);
			LCD_print_str ("     ");
			t2.run = (0x00);
	    }
		
		if (((PINB & (1<<kok)) == 0) & (menu == 0))         //timer1
	    { 
			LCD_Sleep();
	    }
		
		if (((PINB & (1<<kmenu)) == 0) & (menu == 0))         //timer1
	    { 
			 switch (menu)
			 {
				case 0:
					++menu;
					Write_LCD (0x80,0);
	  				LCD_print_str   ("Alarm 1 Modus: ");
					Write_LCD (0x8F,0);
					LCD_out_alarm_sym (alarm_mode_1);
					break;
			 }

		}
	}


	if (timer_menu != 0)
	{
		if (--timer_menu == 0)
		{
			menu = 0;
			LCD_Time();
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
				LCD_out_h();
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

		if (LCD_PWR != 0 & menu == 0) LCD_out_m();
    } 
	
	if (LCD_PWR == 0x00) return;
 
	PORTC |= (1<<p_buzzer); 	

	if (tchar == ':')
	{
		tchar = ' ';
		rchar = 0x00;
	}
	else
	{
		tchar = ':';
		rchar = 0x01;
	}


    if (menu == 0) Write_LCD (th+2,0);
    if (menu == 0) Write_LCD (tchar,1);
  	
	if (t1.run == 1)
	{
		Write_LCD (t1m+2,0);
    	Write_LCD (rchar,1);

		if (++t1.second==60)         //timer1 
	    { 
	        t1.second=0; 
	        if (++t1.minute==60) 
	        { 
	            t1.minute=0; 
	            //if (++t1.hour==24) 
	            //{ 
	            //    t1.hour=0;
	            //}
				//LCD_out_t1h;
 
	        }
			LCD_out_s1();
			LCD_out_m1();			
			if ((t1.minute%alarm_time_1) == 0)
			{
				LCD_out_s1();
				alarm_out(alarm_mode_1,1);

			}
			 
	    } 
		LCD_out_s1();
		LCD_out_m1();
	}
	
	if (t2.run == 1)
	{
		Write_LCD (t2m+2,0);
    	Write_LCD (rchar,1);

		if (++t2.second==60)         //timer2 
	    { 
	        t2.second=0; 
	        if (++t2.minute==60) 
	        { 
	            t2.minute=0; 
	            //if (++t2.hour==24) 
	            //{ 
	            //    t2.hour=0;
	            //}
				//Write_LCD (t2h,0);
				//Write_LCD (t2.hour%10+48,1);
 
	        }
			LCD_out_m2();
			LCD_out_s2();
			if ((t2.minute%alarm_time_2) == 0)
			{
				LCD_out_s2();
				alarm_out(alarm_mode_2,2);
			}

 
	    } 
		LCD_out_s2();
		LCD_out_m2();
	}


} 

ISR(PCINT0_vect)
{
	_delay_ms(60);

	if (((PINB & (1<<k1)) == (1<<k1)) & ((PINB & (1<<k2)) == (1<<k2)) & ((PINB & (1<<kmenu)) == (1<<kmenu)) & ((PINB & (1<<kok)) == (1<<kok))) 
	//if (!(PINB & (1<<k1)) | !(PINB & (1<<k2)) | !(PINB & (1<<kmenu)) | !(PINB & (1<<kok))) 

	{
		timer_key = 0;
		return;
	}
	_delay_ms(30);

	if (((PINB & (1<<k1)) == (1<<k1)) & ((PINB & (1<<k2)) == (1<<k2)) & ((PINB & (1<<kmenu)) == (1<<kmenu)) & ((PINB & (1<<kok)) == (1<<kok))) 
	//if (!(PINB & (1<<k1)) | !(PINB & (1<<k2)) | !(PINB & (1<<kmenu)) | !(PINB & (1<<kok))) 
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

	if ((PINB & (1<<k1)) == 0)         //timer1
    { 
		t1.run ^= (0x01);
		if (!t1.run)
		{
			Write_LCD (t1m+2,0);
	    	Write_LCD (sym_break,1);
		}
		else
		{
			Write_LCD (t1m+2,0);
    		Write_LCD (rchar,1);
		}

    }


	if ((PINB & (1<<k2)) == 0)         //timer2 
    { 
		
	
		t2.run ^= (0x01);
		if (!t2.run)
		{
			Write_LCD (t2m+2,0);
	    	Write_LCD (sym_break,1);
		}
		else
		{
			Write_LCD (t2m+2,0);
    		Write_LCD (rchar,1);
		}
    }

	if ((PINB & (1<<kok)) == 0)         //Select
    { 
		 timer_menu = 20; 
		 switch (menu)
		 {

			case 1:
				if (++alarm_mode_1==3) 
		        { 
		            alarm_mode_1 = 0;
		        }
				Write_LCD (0x8F,0);
				LCD_out_alarm_sym (alarm_mode_1);
				break;

			case 2:
				if (++alarm_time_1==21) 
		        { 
		            alarm_time_1=1;
		        }
				Write_LCD (0x8A,0);
				Write_LCD (alarm_time_1/10+48,1);
				Write_LCD (alarm_time_1%10+48,1);
				break;

			case 3:
				if (++alarm_mode_2==3) 
		        { 
		            alarm_mode_2 = 0;
		        }
				Write_LCD (0x8F,0);
				LCD_out_alarm_sym (alarm_mode_2);
				break;
				break;

			case 4:
				if (++alarm_time_2==21) 
		        { 
		            alarm_time_2=1;
		        }
				Write_LCD (0x8A,0);
				Write_LCD (alarm_time_2/10+48,1);
				Write_LCD (alarm_time_2%10+48,1);
				break;
 
			case 5:
				if (++t.hour==60) 
			    { 
			        t.hour=0; 
			    }
				Write_LCD (0x8B,0);
				Write_LCD (t.hour/10+48,1);
				Write_LCD (t.hour%10+48,1);
				break;
			case 6:
				if (++t.minute==60) 
			    { 
			        t.minute=0; 
			    }
				Write_LCD (0x8E,0);
				Write_LCD (t.minute/10+48,1);
				Write_LCD (t.minute%10+48,1);
				break;
			case 7:
				if (++contrast==4) 
			    { 
			        contrast=1; 
			    }
				Write_LCD (0x8F,0);
				Write_LCD (contrast+48,1);
				comp_contrast(contrast);
				break;
			case 8:

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

	if ((PINB & (1<<kmenu)) == 0)         //Menu
    { 
		 timer_menu = 20; 
		 switch (menu)
		 {
			case 1:
				++menu;
				Write_LCD (0x80,0);
  				LCD_print_str   ("Alarm 1:  00 min");
				Write_LCD (0x8A,0);
				Write_LCD (alarm_time_1/10+48,1);
				Write_LCD (alarm_time_1%10+48,1);

				Write_LCD (0x8D,0);
				break;
			case 2:
				++menu;
				Write_LCD (0x80,0);
  				LCD_print_str   ("Alarm 2 Modus: ");
				Write_LCD (0x8F,0);
				LCD_out_alarm_sym (alarm_mode_2);
				break;
			case 3:
				++menu;
				Write_LCD (0x80,0);
  				LCD_print_str   ("Alarm 2:  00 min");
				Write_LCD (0x8A,0);
				Write_LCD (alarm_time_2/10+48,1);
				Write_LCD (alarm_time_2%10+48,1);

				break;
			case 4:
				++menu;
				Write_LCD (0x80,0);
  				LCD_print_str   ("Stunden:   00:00");
				Write_LCD (0x8B,0);
				Write_LCD (t.hour/10+48,1);
				Write_LCD (t.hour%10+48,1);

				Write_LCD (0x8E,0);
				Write_LCD (t.minute/10+48,1);
				Write_LCD (t.minute%10+48,1);

 				break;
			case 5:

				++menu;
				Write_LCD (0x80,0);
  				LCD_print_str   ("Minuten:   00:00");

				Write_LCD (0x8B,0);
				Write_LCD (t.hour/10+48,1);
				Write_LCD (t.hour%10+48,1);

				Write_LCD (0x8E,0);
				Write_LCD (t.minute/10+48,1);
				Write_LCD (t.minute%10+48,1);
				break;
			case 6:
				++menu;
				Write_LCD (0x80,0);
  				LCD_print_str   ("Kontrast:      ");
				Write_LCD (0x8F,0);
				Write_LCD (contrast+48,1);
				comp_contrast(contrast);

 				break;
			case 7:

				++menu;
				Write_LCD (0x80,0);
  				LCD_print_str   ("Batterie:      V");
				batt=battery_voltage();
				Write_LCD (0x8A,0);
				Write_LCD (batt/100+48,1);
				Write_LCD ('.',1);
				Write_LCD (batt%100/10+48,1);
				Write_LCD (batt%10+48,1);
				break;

			case 8:

				++menu;
				Write_LCD (0x80,0);
  				LCD_print_str   ("Enzo/FW Subingen");
				break;
			
			default:
				menu = 0;
				LCD_Time();
		 }

    }

	timer_key = 2;


	/*
	temp = 0;
	_delay_ms(10);
	while (++temp <= 100)
	{
		_delay_ms(10);
		if ((PINB & 0x1E) == 0x1E)
		{
			temp = 202;
		}

	}

	if (temp < 202)
	*/

	
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
	LCD_out_alarm();

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
	LCD_Time();
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

	LCD_out_alarm();
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

void LCD_out_m1(void)
{
	Write_LCD (t1m,0);
	temp = t1.minute/10 +48;
	if (temp == 48) temp= temp -16;
	Write_LCD (temp,1);
	Write_LCD (t1.minute%10+48,1);
}

void LCD_out_s1(void)
{
	Write_LCD (t1s,0);
  	Write_LCD (t1.second/10+48,1);
	Write_LCD (t1.second%10+48,1);
}

void LCD_out_m2(void)
{
	Write_LCD (t2m,0);
	temp = t2.minute/10 +48;
	if (temp == 48) temp= temp -16;
	Write_LCD (temp,1);
	Write_LCD (t2.minute%10+48,1);
}

void LCD_out_s2(void)
{
	Write_LCD (t2s,0);
  	Write_LCD (t2.second/10+48,1);
	Write_LCD (t2.second%10+48,1);
}

void LCD_out_alarm(void)
{
	Write_LCD (t1al,0);
	LCD_out_alarm_sym (alarm_mode_1);

	Write_LCD (t2al,0);
	
	LCD_out_alarm_sym (alarm_mode_2);	
}

void LCD_out_alarm_sym(unsigned char alarm_mode)
{
	switch (alarm_mode)
	{		
			case 1:
			Write_LCD (sym_speaker,1);
			break;
		case 2:
			Write_LCD (sym_light,1);
			break;
		default:
			Write_LCD ('-',1);
	}
}


void alarm_out(unsigned char alarm_mode, unsigned char alarm_num)
{
	char i;
	int j;
	switch (alarm_mode)
	{		
		case 1:	

					//for(i=0;i<alarm_num;i++)
					//{
						/*PORTC |= (1<<p_buzzer);
						_delay_ms (300);
						PORTC &=~ (1<<p_buzzer);*/
						
						//for (j=0; j< 1200; j++)
						//{
							PORTC &=~ (1<<p_buzzer);
						//	_delay_us (125);
							//PORTC |= (1<<p_buzzer);
							//_delay_us (125);	
						//}

						//_delay_ms (100);
					//}
            
			//break;
		case 2:
					PORTD |= (1<<p_light);
					_delay_ms (200);
					PORTD &=~ (1<<p_light);
					_delay_ms (200);
					if (alarm_num == 2)
					{					
						if (alarm_mode == 1) PORTC |= (1<<p_buzzer);
						PORTD |= (1<<p_light);
						_delay_ms (200);
						PORTD &=~ (1<<p_light); 
						if (alarm_mode == 1) PORTC &=~ (1<<p_buzzer);
						_delay_ms (200);
					}

					if (light != 0) PORTD |= (1<<p_light); 
		break;

			default:
			Write_LCD ('-',1);
	}
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
	Write_LCD (0x83,0);
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
