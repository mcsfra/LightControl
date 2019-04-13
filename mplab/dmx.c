#include <xc.h>
#include "globaldefs.h"
#include "dmx.h"
#include "dimengine.h"

/*
 * Wird aus dem Interrupt heraus aufgerufen;
 * Enthält eine Statemachine zum versenden der Kanalwerte
 */

#define SENDMBB   0    // IDLE period after each complete frame
#define SENDDATA  1    // send one byte of data
#define SENDMAB   2    // IDLE period between BREAK and START slot
#define SENDBREAK 3    // BREAK synchronization signal

#define T100US 156U //(256U-100U) //  preload value for TMR0 to roll over in 100us
#define T60US  196U //(256U-60U)  //  60us value


unsigned char dataCounter; 
unsigned char dmxSendState;

void initDMXSend()  {

    dataCounter  = 0;
    dmxSendState = SENDBREAK;


    // Serial 

    TRISCbits.TRISC6 = 1;        // TX1 Enable
    TRISCbits.TRISC5 = 0;        // Ausgang zum Syncen
    LATCbits.LATC5   = 0;

    BAUDCONbits.BRG16 = 1;       // Baudratengenerator 16-Bit

    SPBRG1            = 15;      // Baudrate einstellen

    TXSTA1 = 0x65;               // See Appnote 1076
   
    RCSTA1 = 0x80;

    // Timer 4 Weil einfacher in der Struktur

    T4CON = 0b00000101;
    IPR5bits.TMR4IP = 1;         // high prio
    PIE5bits.TMR4IE = 1;

//    testLevel = 100; 

}


void sendDMX()  {

  switch( dmxSendState )  {

     case SENDMBB :
          TMR4             = T100US;  // pre-load the timer for 100us BREAK
          TRISCbits.TRISC5 = 0;       // make pin RC5 an output
          LATCbits.LATC5   = 0;       // pull pin RC5 low to force a break condition
          dmxSendState     = SENDBREAK;
          return;

     case SENDDATA:

          if ( dataCounter >= MAXCHANNEL)  {
             TMR4         = T100US;   // pre-load the timer for 100us BREAK
             dmxSendState = SENDMBB;  // proceed to next state SENDMBB
             return;
          }
    
          /*
           * Das ist Quatsch: postincrement und dann index --> kanalverschiebung um eins. Ne?
           * Der Punkt ist allerdings, dass ich alle konfigurationen etc. anpassen müsste, wenn ich das ändere
           * Der Fehler ist in der 1. Version angelegt. Lasse ich so.
           * 25.12.2018 
           */
          dataCounter++;  
          TX1REG = getChannelLevel(dataCounter);
          TMR4 = T60US;
          return;

     case SENDMAB:
          dataCounter  = 0;
          TX1REG       = 0;
          TMR4         = T60US;
          dmxSendState = SENDDATA;
          return;

     case SENDBREAK :
          TRISCbits.TRISC5 = 1;
          TMR4             = T100US;
          dmxSendState     = SENDMAB;
          return;
   }

}



#ifdef UNUSED
void testDMX(void)  {
    
    unsigned char i,j,k; 
    
    unsigned char val; 
    
    for (;;)  {
          
        for ( j = 0 ; j < 255; j++)  {
             for ( i = 0; i < MAXCHANNELS; i++)  {
         
                 val = j; 
                 if ( i ==  1  || i ==  3 || i == 5 || i == 7 ||   // das geht nicht 
                      i ==  15 || i == 14 || i == 13 ||   // bringt einen output
                      i ==  23 || i == 21 ||             // vermutung: das geht wieder nicht
                      i ==  31 || i == 28                // vermutunf : und das geht wieder
                  )  {
                     val = 255U - j; 
                 }
        //         setChannelLevel(i, i % 4 ? 255 - j : j );
                 setChannelLevel(i, val );
             }
        }
    }
} 
#endif







#ifdef DMXRECEIVE

unsigned char dmxState;

void initDMXReceive(void)  {

    BAUDCONbits.BRG16 = 1;     // Baudratengenerator 8-Bit
    BAUDCONbits.CKTXP = 1;     // polaritaet verdrehen

    SPBRG1            = 1;     // Baudrate einstellen
    SPBRGH            = 0;     //

    RCSTA1bits.CREN   = 1;

    RCSTA1bits.SPEN   = 1;

    PIE1bits.RCIE     = 1;
    IPR1bits.RCIP     = 1;

    dmxState      = DMX_NOTSYNCED;

}

unsigned char errflag;
unsigned char received;
unsigned char currentAdress = 0x00;


/*
 * Empfangsroutine brauche ich eigentlich nicht
 */

void handleDMX()  {

   /*
   ** error-flag holen ( muss vor dem lesen der eigentlichen date gemacht werden )
   */
   errflag = FERR;
  /*
   ** Das Empfangene Byte holen
   */
   received = RCREG;

   if ((errflag == 1) && ( dmxState == DMX_NOTSYNCED)  )  {
       dmxState = DMX_WAITSTART;
       return;
   }

   if ( (dmxState == DMX_WAITSTART) && (received == 0x00))  {
      dmxState = DMX_PAYLOAD;
      return;
   }

   if ( dmxState == DMX_PAYLOAD)  {

       /*
        *   Handling of received bytes
        */
      currentAdress++;

      if ( currentAdress >= 128)
         dmxState = DMX_NOTSYNCED;
   }
}

#endif
