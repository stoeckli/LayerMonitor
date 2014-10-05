//****************************************************************
//
//  Funktionen für das Display EA-DOGM 163 W-A
//
//  21.04.2005
//
//  Erstellt von Huber Rainer
//
//*****************************************************************

/*
!!! Instruction-Befehle !!!

!! Vor bestimmten Befehle in die richtige Instruction-Tabelle springen !!

Grundbefehle:    
    
    DOG_Instruction(0x01);          //Clear LCD, (+Cursor home)
    DOG_Instruction(0x02);          //Return Home

Cursorbefehle:  (Set DDRAM-Adress)
    
    Oberstes Bit(2^8 muss immer gesetzt sein, danach je Zeile von 0x00-0x0F bzw.0x80-0x8F)
    Dies für Zeile1. Zeile 2: 0x90-0x9F     Zeile 3: 0xA0-0xAF
    
    DOG_Instruction(0x80);          //Cursor Zeile 1 Platz 1
    DOG_Instruction(0x81);          //Cursor Zeile 1 Platz 2
    DOG_Instruction(0x82);          //Cursor Zeile 1 Platz 3
    .                 .              .                   .
    .                 .              .                   .
    DOG_Instruction(0x8E);          //Cursor Zeile 1 Platz 15
    DOG_Instruction(0x8F);          //Cursor Zeile 1 Platz 16
    DOG_Instruction(0x90);          //Cursor Zeile 2 Platz 1
    DOG_Instruction(0x9F);          //Cursor Zeile 2 Platz 16
    DOG_Instruction(0xA0);          //Cursor Zeile 3 Platz 1
    DOG_Instruction(0xAF);          //Cursor Zeile 3 Platz 16

Display ON/OFF 
    
    0   0   0   0   1   DisplayON/OFF   CursorON/OFF    SegmentBlinken
    7   6   5   4   3       2                 1                 0
    
    DOG_Instruction(0x0E);          //Segment-Blinken aus
    DOG_Instruction(0x0C);          //Cursor aus und Blinken aus
    DOG_Instruction(0x08);          //Display aus (Daten bleiben erhalten)

Instruction Tables:
    
    Table 0 : DOG_Instruction(0x28);
    Table 1 : DOG_Instruction(0x29);
    Table 2 : DOG_Instruction(0x2A);

Cursor or Display Shift

    Instruction Table 0
    DOG_Instruction(0x28);
    dann    
    DOG_Instruction(0x10);  //Shift Cursor to left
    DOG_Instruction(0x14);  //Shift Cursor to right
    DOG_Instruction(0x18);  //Shift Dislpay to left
    DOG_Instruction(0x1C);  //Shift Dislpay to left
*/

//  ----------------------------------------------------------------------------------------
// Diese Funktion ist eine Unterfunktion der Initialisierung des 
// Displays. Es werden hier nur 4 Bits übertragen und danach findet
// eine mitgegebene Wartezeit statt.

#include <util/delay.h>
#include <avr/io.h>
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

void DOG_LCD_High_Nib (char Bitkombination, int Zeit){

    char Zwischenspeicher = 0;

    LCDPNULL;                                   //oberen 4 Bits von Port 5 löschen
    Zwischenspeicher = (Bitkombination & 0xF0); //unteren 4 Bits des Zeichens ausblenden
    DOG_RS_0;
    DOG_RW_0;
    DOG_ENABLE_1;
    LCD_PORT |= Zwischenspeicher;               //Bits auf den Port legen
    DOG_ENABLE_0;
    LCDPNULL;                                   //oberen 4 Bits von Port 5 löschen
    _delay_ms(Zeit);                            //wichtige Wartezeit
    
}//Funktion Ende


//--------------------------------------------------------------------------------------------
// Diese Funktion ist eine Unterfunktion der Initialisierung des 
// Displays. Es wird hier ein ganzes Byte nacheinander in 2 Nibbles
// übertragen (erst höherwertiges Nibble, dann niederw. Nibble).
// Hirbei handelt es sich um Daten für das Instruction-Register 

void DOG_Instruction (char Bitkombination){
    
    char Zwischenspeicher = 0;
    
    LCDPNULL;                                   //oberen 4 Bits von Port 5 löschen
    Zwischenspeicher = (Bitkombination & 0xF0); //unteren 4 Bits des Zeichens ausblenden
    Bitkombination = Bitkombination >> 4;
	DOG_RS_0;
    DOG_RW_0;
    DOG_ENABLE_1;
    LCD_PORT |= Zwischenspeicher;               //Bits auf den Port legen
    DOG_ENABLE_0;
    LCDPNULL;                                   //oberen 4 Bits von Port 5 löschen
    Bitkombination = (Bitkombination & 0x0F);   //oberen 4 Bits des Zeichens löschen
    Zwischenspeicher = (Bitkombination & 0x0F); //unteren 4 Bits des Zeichens ausblenden
    DOG_ENABLE_1;
    LCD_PORT |= Zwischenspeicher;               //Bits auf den Port legen
    DOG_ENABLE_0;
    LCDPNULL;                                   //oberen 4 Bits von Port 5 löschen
    _delay_ms(50);                                  //wichtige Wartezeit

}//Funktion ENDE 

//-------------------------------------------------------------------------------------------
// Funktion, um das Display zu initialisieren

void DOG_LCD_INIT (void){

    DOG_RS_0;                       //RS-Leitung auf 0
    DOG_RW_0;                       //R/W-Leitung auf 0
    DOG_ENABLE_0;                   //Enable-Leitung auf 0
    DOG_LCD_High_Nib(0x30,3000);    //Senden des Function set I
    DOG_LCD_High_Nib(0x30,50);     //Senden des Function set II
    DOG_LCD_High_Nib(0x30,50);     //Senden des Function set III
    DOG_LCD_High_Nib(0x20,50);     //Senden des Function set IV
    DOG_Instruction(0x29);       //Senden des Function set V
    DOG_Instruction(0x15);       //Senden des Internal OSC frequency
    DOG_Instruction(0x79);       //Senden des Contrast SET
    DOG_Instruction(0x55);       //Senden des Power/ICON/Conrast control
    DOG_Instruction(0x6E);       //Senden des Follower control
    DOG_Instruction(0x0F);       //Senden des Display ON/OFF control
    DOG_Instruction(0x01);       //Clear LCD, Cursor home
    DOG_Instruction(0x06);       //Cursor Auto Increment
    
}//Funktion ENDE


//------------------------------------------------------------------------------------------
// Funktion zur Kontrasteinstellung 
// Voreinstellung des Kontrast bei der Initialisierung des Dog LCDs

void DOG_Contrast (char a){

    extern char oberes_Kontrast_Byte;   //voreingestellt im Hauptprogramm mit 9
    extern char unteres_Kontrast_Byte;  //voreingestellt im Hauptprogramm mit 1

    if (a == 2){                    //wenn 2 übergeben wird, Kontrast erhöhen
        unteres_Kontrast_Byte ++;
        if ((unteres_Kontrast_Byte & 0x0F) == 0x00){//wenn Überlauf des unteren Nibble
            oberes_Kontrast_Byte ++;        
        }        
    }  
    if (a == 1){                    //wenn 1 übergeben wird, Kontrast erhöhen
        unteres_Kontrast_Byte --;        
        if ((unteres_Kontrast_Byte & 0x0F) == 0x0F){//wenn Unterschreitung des unteren Nibble
            oberes_Kontrast_Byte --;
        }
    }   
    unteres_Kontrast_Byte = (unteres_Kontrast_Byte & 0x0F);//Maskierung
    unteres_Kontrast_Byte = (0x70 | unteres_Kontrast_Byte);//Überlagerung mit Intructions-Byte  
    
    oberes_Kontrast_Byte = (oberes_Kontrast_Byte & 0x03);//Maskierung
    oberes_Kontrast_Byte = (0x54 | oberes_Kontrast_Byte);//Überlagerung mit Intructions-Byte  
    
    DOG_Instruction(0x29);                              //Table 1
    DOG_Instruction(unteres_Kontrast_Byte);             //Übertragung des Kontrast-Bytes
    DOG_Instruction(oberes_Kontrast_Byte);              //Übertragung des Kontrast-Bytes
    
}//Funktion ENDE


//-------------------------------------------------------------------------------------------------
// Funktion zur Zusammenlegung von zwei Zeilen

void DOG_Double_High (char a){

    if (a == 1){                //oberen beiden Zeilen Zusammen
        DOG_Instruction(0x2A);  //Table2
        DOG_Instruction(0x18);  //UD auf 0
        DOG_Instruction(0x2C);  //DH auf 1
    }
    if (a == 2){                //unteren beiden Zeilen Zusammen
        DOG_Instruction(0x2A);  //Table2
        DOG_Instruction(0x10);  //UD auf 1
        DOG_Instruction(0x2C);  //DH auf 1
    }
    else{
        DOG_Instruction(0x28);  //3 Zeienbetrieb
    }
}//Funktion ENDE


//-------------------------------------------------------------------------------------------
// Es wird hier ein ganzes Byte - ein ASCII Zeichen - nacheinander in 2 Nibbles
// übertragen (erst höherwertiges Nibble, dann niederw. Nibble).
// Hirbei handelt es sich um Daten für das anzuzeigende Daten-Register
// Für Datenregister RS auf 1!

void show_ASCII (char Bitkombination){
    
    char Zwischenspeicher = 0;
    
    LCDPNULL;                                   //oberen 4 Bits von Port 5 löschen
    Zwischenspeicher = (Bitkombination & 0xF0); //unteren 4 Bits des Zeichens ausblenden
    Bitkombination = Bitkombination >> 4;           //4 Stellen nach unten schieben
	DOG_RS_1;
    DOG_RW_0;
    DOG_ENABLE_1;
    LCD_PORT |= Zwischenspeicher;               //Bits auf den Port legen
    DOG_ENABLE_0;
    LCDPNULL;                                   //oberen 4 Bits von Port 5 löschen
    Bitkombination = (Bitkombination & 0x0F);   //oberen 4 Bits des Zeichens löschen
    Zwischenspeicher = (Bitkombination & 0x0F); //unteren 4 Bits des Zeichens ausblenden
    DOG_ENABLE_1;
    LCD_PORT |= Zwischenspeicher;               //Bits auf den Port legen
    DOG_ENABLE_0;
    LCDPNULL;                                   //oberen 4 Bits von Port 5 löschen
    _delay_ms(50);

}//Funktion ENDE


// ----------------------------------------------------------------------------------------------
// Hier wird der Text der vorher in den LCD_Puffer gelegt wurde mit der
// mitgegebenen Zeile angezeigt.

void DOG_TEXT (char DOG_Zeile){

    extern char LCD_Puffer[15];
    char i;
    
    if (DOG_Zeile < 4){
        if (DOG_Zeile == 1){
            DOG_Instruction(0x80);       //Senden der Cursor Adresse
        }
        if (DOG_Zeile == 2){
            DOG_Instruction(0x90);       //Senden der Cursor Adresse
        }
        if (DOG_Zeile == 3){
            DOG_Instruction(0xA0);       //Senden der Cursor Adresse
        }
    }
    else{
        DOG_Instruction(0x80);       //Senden der Cursor Adresse    
    }
    
    for (i=0;i<=15;i++){            //eigentliches Senden des Textes
        show_ASCII(LCD_Puffer[i]);    
    }
}//Funktion ENDE
