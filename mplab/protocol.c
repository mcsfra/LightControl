#include <xc.h>
#include "protocol.h"
#include "buttons.h"

/*
** Implementiert das eingehende RS485 Protokoll
*/


static unsigned char protocolState;
static unsigned char device = 0;
static unsigned char button = 0;


#define PSTATE_INIT 0
#define PSTATE_PAYLOAD 1
#define PSTATE_CHECKSUM 2



void initButtonProtocol(void)  {

    protocolState = PSTATE_INIT;

}


void buttonByteReceived(unsigned char b)  {
   
    unsigned char checksum;
    unsigned char code;

    /*
    ** Sind wir am Anfang? Dann hätten wir auch gerne ein 0xF1 als startbyte
    ** Wenn das Gut ist, schalten wir weiter.
    */

    if (protocolState == PSTATE_INIT && b == 0xF1U )  {
        protocolState = PSTATE_PAYLOAD;
        return;
    }

    if ( protocolState == PSTATE_PAYLOAD )  {

       device = ((b >> 4U) & 0x0FU);   // warning. Why?
       button = b & 0x0FU;
       protocolState = PSTATE_CHECKSUM;
       return;
    }

    if ( protocolState == PSTATE_CHECKSUM )  {

        code     = ((device << 4U) & 0xF0U) | (button & 0x0FU);
        checksum = 0xF1U ^ code ;

        if ( checksum == b)  {

            decodeButton(code);
            
        }
    }
    protocolState = PSTATE_INIT;
}

// EOF