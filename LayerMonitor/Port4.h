
//  Port 4
#include	<avr/io.h>
#define LCD_Bel_aus         (P4OUT |= 0x04)
#define LCD_Bel_an          (P4OUT &= ~0x04)
#define DOG_RW_1            (P4OUT |= 0x06)
#define DOG_RW_0            (P4OUT &= ~0x06)
#define DOG_ENABLE_1        (P4OUT |= 0x07)
#define DOG_ENABLE_0        (P4OUT &= ~0x07)
#define DOG_RS_1            (P4OUT |= 0x05)
#define DOG_RS_0            (P4OUT &= ~0x05)
#define LCDPNULL            (P4OUT &= ~0x0F)    //löscht die oberen 4Bits (vorbereitung fürs schreiben)
#define LCD_PORT            P4OUT
#define P4OUT				PORTD
