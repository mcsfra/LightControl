#include <xc.h>        
#include "buttons.h"
#include "massStorage.h"
#include "fifo.h"
#include "protocol.h"
#include "scenes.h"


#define MAXBUTTONS 64

/*
 *  Taster-Aus Hysterese.
 *  In Kunjunktion mit dem Timer-Preload (Timer0) und der Geschwindigkeit der Seriellen Schnittstelle
 *  Bei 31Kbaud habe ich eine merkliche latenz.
 *
 *  Abzuwägen ist zwischen latenz und schalterflattern.
 */

#define SCHWELLWERT 3

BUTTON buttons[MAXBUTTONS];

unsigned char lastPressedKey = 0x00;

/*
 * Damit wir feststellen koennen, ob ein Button n i c h t  m e h r gedrueckt ist,
 * benoetigen wir einen timer, hier stellen wir dann fest, ob ein watchdogevent eingetreten ist
 */

void initButtonHardware(void)  {

    /*
    ** Tastencodersignale werden auf EUSART2 empfangen
    */

    TRISDbits.TRISD7 |= 1;      // RX2 Ein

    RCSTA2 = 0;

    BAUDCON2bits.BRG16 = 0;     // Baudratengenerator 8-Bit
    BAUDCON2bits.DTRXP = 1;     // polaritaet verdrehen - ORIGINAL
  //  BAUDCON2bits.DTRXP = 0;     // polaritaet verdrehen - Ohne Max488
        
    RCSTA2bits.SPEN = 1;   // und einschalten
    RCSTA2bits.CREN = 1;

    SPBRG2 = 7;

    IPR3bits.RC2IP = 1;     // High Priority
    PIE3bits.RC2IE = 1;
  
    /*
     * Timer0 Bildet den Button-Watchdog
     * Wenn
     */

    T0CONbits.TMR0ON = 0;

    TMR0H = 0;
    TMR0L = 0;
    INTCONbits.TMR0IF = 0;
 
    T0CONbits.T08BIT  = 0;    // 16 Bit
    T0CONbits.T0CS    = 0;    // INput Source: FOSC/4
    T0CONbits.PSA     = 0;    // Use Prescaler (nagative logik. grrr)

    T0CONbits.T0PS    = 0;    // Prescaler 1:16

    /*
    ** Timer Interupt einschalten
    */
    RCONbits.IPEN      = 1;   // Periphere Interrupts ein
    INTCONbits.PEIE    = 1;   // das sollte garnicht nötig sein, oder?
    INTCON2bits.TMR0IP = 0;   // Low-Prio
    INTCONbits.TMR0IE  = 1;   // Interrupt für Timer ein
  
    /*
    ** und nun einschalten
    */
    T0CONbits.TMR0ON = 1;

}

void testRC2Errors()  {
    
    if ( RCSTA2bits.FERR == 1)  {
        RCSTA2bits.SPEN = 0;  
        initButtonProtocol(); 
        RCSTA2bits.SPEN = 1;  
    }
    
    if ( RCSTA2bits.OERR == 1)  {
        RCSTA2bits.SPEN = 0;
        fifoInit(); 
        initButtonProtocol(); 
        RCSTA2bits.SPEN = 1;  
    }
    
};


/*
 * Findet anhand der Knopfnummer die entsprechende Datenstruktur
 */
BUTTON *findButton(BUTTONNUMBER code)  {

    BUTTONINDEX i;

    lastPressedKey = (unsigned char)code;

    for ( i = 0; i < MAXBUTTONS; i++)  {

        if ( buttons[i].buttonNumber == code)
            return &buttons[i];
    }
    return (BUTTON*) 0;
}

/*
 * Holt die Buttonmatrix aus dem Flash und legt benötigte Daternstrukturen an
 */

void initAllButtons()  {

    unsigned char buttonIndex    = 0;   // laufender index
 
    unsigned char data[2];  // 0 - buttonProgram, 1 - buttonMode
    unsigned char i;

    for ( i = 0; i < 0xFF; i++)  {
      readAButton(i, data );

      if ( data[0] != 0)  {
          initButton(buttonIndex, i, data[0], data[1]);
          buttonIndex++;
      }
    }
}

/*
** Legt EINEN Button im Ram an und initialisiert die erforderlichen datenstrukturen
*/
void initButton(BUTTONINDEX i, BUTTONNUMBER bn, ASSIGNEDSCENE as, unsigned char buttonMode)  {

    BUTTON *bp = &buttons[i];

    bp-> buttonNumber  = bn;      // device und nummer des Knopfs
    bp-> assignedScene = as;     //

    bp-> buttonMode    = buttonMode == 1U ? 1U : 0;  // unsigned char nach bit. Brauche ich das so?  
    
    bp-> isActive      = 0;      // ist der knopf akuell als aktiv erkannt ?
    bp-> watchdog      = 0;      // zähler

}

/*
 * Ich möchte das mit den Knöpfen anders gestalten:
 * 
 * ich möchte aus den Bursts ein kontinuierliches "Signal" machen.
 * das Bedeutet: 
 * 1. Liefert der Buttonburst keine zeitpasis meht zum Dimmen, das dimmen hat siene Eigene und die soll verwendet werden
 * 2. Flankenwechsel werden in einer weiteren schleife ausgewertet und ensprechend behandelt.
 *    Das wird den code entsprechend entzerren
 *
 *  Weitere anmerkungen zur DIM engine: 
 * 
 *  Ich brauche hier eine art eventhandling: wenn die "Endlage" einer szene erreicht ist, 
 *  soll ein entsprechender event ausgelöst werden
 *  der eventhandler wird dann je nach modus die dimrichtung umdrehen bzw. neu berechnen.
 *
 *  So kann es gehen:
 * 
 *  Der Buttonhandler schreibt "events" in ein szenenregister ( array fpr alle szenen) 
 *  Die events werden in einem eigenen handler ausgewertet.
 * 
 */


/*
 * Findet den jeweiligen Button und führt die zugewiesene Funktion aus
 *
 * Hier ist die stelle, an der der Buttonmodus aufgerufen wird
 * 1. Old Fashioned: Ein Programm wird aufgerufen und entsprechend wird das dimmen gestartet
 * 2. Continuous Mode: Kurz Drücken: ein, gedrückt halten: hoch dimmen bis hell dann runter usw.
 * 
 *      Mir fallen diverse dim-modi ein
 * 
 *      *   hochdimmen, bis die erste funzel den maximalwert erreicht. dann runter
 *      *   - linear oder proportional
 *      *   - incremente so dass zeitgleich maximale helligkeit erreicht wird.
 */

void decodeButton(unsigned char code)  {

    BUTTON *bp = findButton((BUTTONNUMBER)code);

    if (bp == 0)
        return;
   
    incrementSceneCounter(bp->assignedScene);
    if ( bp-> isActive == 0)  {
       handleSceneOn(bp->assignedScene, bp -> buttonMode);
    }
    
    bp-> isActive = 1;
    bp-> watchdog = 0;
}

void buttonsReset(void)  {

   unsigned char i;

   BUTTON *bp = buttons;

   for ( i = 0; i < MAXBUTTONS; i++)  {

      if (bp-> watchdog > SCHWELLWERT)  {
          bp-> isActive = 0;
          bp-> watchdog = 0; 
          handleSceneOff(bp -> assignedScene, bp -> buttonMode); 
      }
      else  {
         if (bp-> isActive)   {
             bp-> watchdog++;
         }
      }
      bp++;
   }
}

/*
 * Setzt für einen Button eine entsprechende Aktion.
 * Wenn Aktion == 0 fliegt der Button aus der Konfiguration
 *
 * ich möchte mir gerne die verwaltung von dynamischen arrays ersparen;
 * deswegen wird nach der registrieung einfach die komplette struktur neu aufgebaut
 */

void registerButton(unsigned char button, unsigned char action, unsigned char buttonMode)  {

    storeAButton(button, action, buttonMode );
    initAllButtons();
}




/*
 *  Erste Seite des EEPROM-Speichers enthält für jeden Taster die zugeordnete Szene
 */


void storeAButton(unsigned char button, unsigned char program, unsigned char mode)  {
     HDByteWriteI2C(0xA0, 0, button, program );
     HDByteWriteI2C(0xA0, 1, button, mode );
     
}

void readAButton(unsigned char button, unsigned char *cp)  {
    
    unsigned char data = 0;                      // Zuweisung ist eigentlich überflüssig, unterdrückt aber ein warning
    HDByteReadI2C( 0xA0,0, button, &data, 1 );
    *cp++ = data; 
    
    HDByteReadI2C( 0xA0,1, button, &data, 1 );
    *cp = data; 
}


unsigned char getLastPressedKey(void)  {
    return lastPressedKey;
}


// EOF
