#include <xc.h>
#include "hardware.h"

void initHardware(void)  {

  // 16MHz internal OscillatorBlock
    
  OSCCONbits.IDLEN = 0;

  OSCCONbits.IRCF2 = 1;
  OSCCONbits.IRCF1 = 1;
  OSCCONbits.IRCF0 = 1;

  OSCCONbits.SCS1  = 1;
  OSCCONbits.SCS0  = 0;

  // PLL Einschalten
  // Scheint nicht zu wirken.
   OSCTUNEbits.PLLEN = 1;
  // Aktuell laufen wir mit 16Mhz

  /*
  ** I/O
  */

  CM1CON0  = 0;
  CM2CON0  = 0;


  LATA    = 0x00;
  PORTA   = 0x00;
  ANSELA  = 0x00; 
  TRISA   = TRISAMASK;             

  
  PORTB   = 0x00;
  LATB    = 0x00;
  ANSELB  = 0x00;
  TRISB   = TRISBMASK;
  TRISBbits.RB0 |= 1;
  TRISBbits.RB1 |= 1;
 
  PORTC   = 0x00;
  LATC    = 0x00;
  ANSELC  = 0x00;
  TRISC   = 0xFF;
 
  ANSELD = 0x00;

  TRISD  = TRISDMASK;
}
