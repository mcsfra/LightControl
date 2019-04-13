#include <xc.h>        /* XC8 General Include File */

#include "hardware.h"
#include "fifo.h"
#include "dmx.h"
#include "protocol.h"
#include "buttons.h"
#include "dimengine.h"
#include "scenes.h"
#include "massStorage.h"
#include "i2cslave.h"


volatile unsigned char bDoDimStep   = 0;
volatile unsigned char bButtonReset = 0;
volatile unsigned char bSPISlave    = 0;

volatile unsigned char bI2CReadPending = 0; 

extern CHANNEL newChannels[ENGINE_MAXCHANNEL];


// wird von __delay_ms benoetigt
#define _XTAL_FREQ 8000000


#define PRODUCTION 1

void main(void)
{
    /*
     *  Sowohl Master als aus Slave (also das hier) hängen an derselben Stromversorgung
     *  Es hat sich gezeigt, dass diese software nicht startet, wenn beide teile
     *  gleichzeitig bestromt werden. 
     *  Diese 3 delays helfen. 
     */


    
    
    initHardware();     // Allgemeines
    
#ifdef PRODUCTION        // ich mag das im labor nicht haben.
       __delay_ms(5000);
 #endif
  
    
    initDimEngine();        // neu
    initSceneEvents(); 
       
    fifoInit();
    initI2CParser(); 
            
  
    initI2CSlave();     // Kommunikation
    initI2C();          // Für Speicher
    
    initButtonProtocol();
    initButtonHardware();

 
    
    initAllButtons();             // überträgt den Flah-inhalt ins ram
    // ^^ Hier passiert was was die initialisierung von der dimengine und sceneevents ruiniert. 
    initDimEngine();         
    initSceneEvents();     // 
  
    initSceneChangeHardware();   // Timer zur dimsteptaktung
    
    initDMXSend();
    
    RCONbits.IPEN   = 1;
    INTCONbits.GIEH = 1; 
    INTCONbits.GIEL = 1;
    INTCONbits.PEIE = 1;    

    
    for (;;)  {
 
        if (!fifo1Empty())  {
          buttonByteReceived(fifo1Get());
        }

        testRC2Errors();

        while ( !fifo2Empty())  {
          i2cParser(fifo2Get());
        }
        
        if( bDoDimStep == 1)  {
            dimEngineStep(); 
            bDoDimStep = 0;
        }

        if( bButtonReset == 1)  {
            buttonsReset();
            bButtonReset = 0;
        }
  
        i2cSendRawData();       
      
   
    }   // main loop

}


