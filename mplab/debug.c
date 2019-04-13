#include <xc.h>        /* XC8 General Include File */
//#include <p18cxxx.h> 
#include "debug.h"



/*
** Local Prototypes 
*/

void noppi(void); 

/*
** Functions
*/

#ifdef NOTUSED
void debugInit()  {
     TRISAbits.TRISA0 = 0; 
     TRISAbits.TRISA1 = 0; 
     TRISAbits.TRISA2 = 0; 

}


/*
** lowlevel Debugging: pin-toggling
*/

/**/
void noppi()  {

   unsigned int i; 
   for ( i = 0; i < 0x00FF; i++)  {
      Nop();
   }
}

void toggle0(void)  {
   CHANNEL0 =  CHANNEL0 == 0 ? 1U : 0; 
}

void toggle1(void)  {
   CHANNEL1 =  CHANNEL1 == 0 ? 1U : 0; 
}

void toggle2(void)  {
   CHANNEL2 =  CHANNEL2 == 0 ? 1U : 0; 
}


void set0(void)  {
   CHANNEL0 = 1; 
}

void set1(void)  {
   CHANNEL1 = 1; 
}

void set2(void)  {
   CHANNEL2 = 1; 
}

void reset0(void)  {
   CHANNEL0 = 0; 
}

void reset1(void)  {
   CHANNEL1 = 0; 
}

void reset2(void)  {
   CHANNEL2 = 0; 
}




void ping0(unsigned char numPings)  {
 
   unsigned int i; 

   while(numPings--)  {
      CHANNEL0 = 1;
      noppi(); 
      CHANNEL0 = 0; 
      noppi(); 
   }
}

void ping1(unsigned char numPings)  {

   while(numPings--)  {
      CHANNEL1 = 1;
      noppi(); 
      CHANNEL1 = 0; 
      noppi(); 
   }
}

void ping2(unsigned char numPings)  {

   while(numPings--)  {
      CHANNEL2 = 1;
      noppi(); 
      CHANNEL2 = 0; 
      noppi(); 
   }
}



#endif



