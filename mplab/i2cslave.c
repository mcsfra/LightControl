#include <xc.h>
#include "i2cslave.h"
#include "scenes.h"
#include "buttons.h"
#include "dimengine.h"
#include "massStorage.h"
#include "commands.h"
#include "fifo.h"

unsigned char bufCount = 0;

/*
 * Hier merke ich mir, ob nach dem Aufrufen einer Szene ein Kanal in der Intensität geändert wurde.
 */
volatile unsigned char bSceneChanged = 0;

volatile unsigned char bStoreDone = 0;  // obsolet???

/*
 * Statemachine States
 */

#define SPISTATE_IDLE 0
#define SPISTATE_DATA 1
#define SPISTATE_CHKS 2   //Checksum
#define SPISTATE_ACK  3   //ACKNOWLEDGE


/*
 * Commands
 */

volatile unsigned char spiState;

volatile unsigned char spiCommand;

unsigned char spiCount;
unsigned char spiChecksum;
unsigned char spiData[64];

unsigned char dummy = 0;

unsigned char testLevel;

/*
 * raw data send
 */

unsigned char bGetDataBlockActive = 0;
unsigned char dataBlockCounter; 
unsigned char dataBlockPage; 
  
/*
 *  Forward declarations
 */

unsigned char testProtocolHeader(unsigned char numBytes);

void initI2CParser()  {
    
    unsigned char i;
    
    for ( i = 0; i < 64; i++)   {
        spiData[i]  = 0x00;
    }
    spiCount    = 0x00; 
    spiChecksum = 0x00; 
}



void initI2CSlave()  {

    unsigned char i;
    
    spiState    = SPISTATE_IDLE;   
    spiCount    = 0;
    spiChecksum = 0;
    
    LATDbits.LATD0   = 0;     // PIN 19 - SCL2
    LATDbits.LATD1   = 0;     // PIN 20 - SDA2

    PORTDbits.RD0    = 0;
    PORTDbits.RD1    = 0;

    ANSELD           = 0;
    TRISDbits.TRISD0 = 1;
    TRISDbits.TRISD1 = 1;
   
     SSP2CON2 = 0x00;       // see Page 262 - 18f45k22.pdf
                            // 7  -- General Call  --> 0 : Disabled
                            // 6  -- Acknowledge received Readonly --> 0
                            // 5  -- Acknowledge Data Bit  das weiss ich nicht 
                            // 4  -- Only Master mode
                            // 3  -- receive enable. master only
                            // 2  -- mastr only  
                            // 1  -- mater only 
                            // 0  -- master only
     
     SSP2CON2bits.SEN2 = 1; 
     
     
     SSP2CON3 = 0x08;       // seet Page 264 - 18f45k22.pdf
     SSP2MSK  = 0xFE;       
     SSP2STAT = 0x00;       
     SSP2ADD  = 0x22;       // i2c Adress - page 265 --> 0x11
     SSP2CON1 = 0x026;      // See Page 260 - 18f45k22.pdf     

     PIR3bits.SSP2IF = 0;
     IPR3bits.SSP2IP = 1;
     PIE3bits.SSP2IE = 1;
  
     PEIE = 1; 
}

unsigned char myCounter = 0;
unsigned char help;

SCENECHANNEL sc; 

/*
 * 2. Version des Parsers
 */
void i2cParser(unsigned char data)  {
    
    /*
     * Die Kommunikation läuft Seitens des Clients Streng synchron. 
     * Ergo darf hier nichts mehr im Sendebuffer sein. 
     * Ich mach ihn also leer.
     */
//    while( !fifo3Empty())  {
//        fifo3Get(); 
//    }
    
    if (spiCount == 0)  {
        spiChecksum = data; 
    }
    else  {
        /*
         * Wenn das Datum die checksumme ist ergibt sich nun in spiChecksum 0x00! 
         */
        spiChecksum ^= data; 
    }
    
    spiData[spiCount++] = data; 
    
    switch ( spiData[0] )  {
        
        case I2CCMD_SETCHANNEL: 
             if ( testProtocolHeader(4))  {
                  bSceneChanged = 1;
                  setChannelLevel(spiData[1],spiData[2]);
                  fifo3Put('!'); 
             }
             break;
            
        case I2CCMD_SETSCENELEVEL: 
             if ( testProtocolHeader(4))  {
                  bSceneChanged = 1;
                  setSceneLevel(spiData[1],spiData[2]);
                  fifo3Put('!'); 
             }
             break;
 
        case I2CCMD_GETCHANNEL: 
             
             if ( testProtocolHeader(3))  {
      
                help = getCurrentScene();
                 
                readSceneChannel(help, spiData[1], &sc);
                fifo3Put('!'); 
                data = sc.isTransparent; 
                spiChecksum = data; 
                fifo3Put(data);  
               
                data =  getChannelLevel(spiData[1]); 
                spiChecksum ^= data; 
                fifo3Put(data);  
               
                data = sc.level; 
                spiChecksum ^= data; 
                fifo3Put(data);  
               
                data = sc.dimSpeed; 
                spiChecksum ^= data; 
                fifo3Put(data);  
                  
                fifo3Put(spiChecksum);
              
             }
             break; 
        case I2CCMD_GETLIVECHANNELVALUE:
             if ( testProtocolHeader(3))  {
                 data =  getChannelLevel(spiData[1]); 
                 fifo3Put('!'); 
                 fifo3Put(data);  
                 fifo3Put(data ^ 0xFFU);
             }
             break; 
 
        case I2CCMD_CALLSCENE : 
             if ( testProtocolHeader(3))  {
                  callScene(spiData[1]);
                  bSceneChanged = 0;
                  fifo3Put('!'); 
             }
             break;

        case I2CCMD_GETCURRENTSCENE :
             if ( spiCount == 2 )  {     // Das ist DOOF wegen 0xFF
                if ( spiChecksum == 0xFF)   {  // es kommt rein: kommando und ^kommando das ergibt 0xFF
                    fifo3Put('!');
                    data = getCurrentScene();
                    fifo3Put(data);  
                    fifo3Put(0xFFU^data);  
                }
                else  {
                    fifo3Put('?');
                }   
                spiCount = 0;
             }
             break; 
        case I2CCMD_SETSCENEFORKEY  :
             if ( testProtocolHeader(5))  {
                  bStoreDone = 1;
                  registerButton(spiData[1],spiData[2], spiData[3]);  
                  fifo3Put('!');
             }
             break; 
 
        case I2CCMD_GETSCENEFORKEY :
             if ( testProtocolHeader(3))  {
                 readAButton(spiData[1], &spiData[2]);
                 fifo3Put('!'); 
                 spiChecksum = spiData[2];  
                 spiChecksum ^= spiData[3];  
                 fifo3Put(spiData[2]); 
                 fifo3Put(spiData[3]); 
                 fifo3Put(spiChecksum);  
             }
             break; 
 
        case I2CCMD_GETSCENECHANGE :
              if ( spiCount == 2 )  {    // DOOF: Wegen falschrummer checksumme
                if ( spiChecksum == 0xFF)   {  // es kommt rein: kommando und ^kommando das ergibt 0xFF
                    fifo3Put('!');
                    data = bSceneChanged == 0 ? 'N' : 'Y';
                    fifo3Put(data);  
                    fifo3Put(0xFFU^data);  
                }
                else  {
                    fifo3Put('?');
                }   
                spiCount = 0;
             }
             break; 
        case I2CCMD_GETLASTKEY      :
              if ( spiCount == 2 )  {   // DOOF
                if ( spiChecksum == 0xFF)   {  // es kommt rein: kommando und ^kommando das ergibt 0xFF
                    fifo3Put('!');
                    data = getLastPressedKey();
                    fifo3Put(data);  
                    fifo3Put(0xFFU^data);  
                }
                else  {
                    fifo3Put('?');
                }   
                spiCount = 0;
              }
              break; 
        case I2CCMD_BLACKOUT:  
            
             if ( testProtocolHeader(3))  {
                  doBlackout(spiData[1]);
                  fifo3Put('!'); 
             }
             break;
  
        case I2CCMD_STORECURRENTSETTING:     
             if ( testProtocolHeader(3))  {
                 storeCurrentLevelsAsScene(spiData[1]);   
                 fifo3Put('!'); 
             }
             break;
            
        case I2CCMD_CHANNELMASK: 
             if ( testProtocolHeader(19))  {
                 storeChannelmask(spiData[1], &spiData[2]);
                 fifo3Put('!');
             }
             break; 
 
        case I2CCMD_GETCHANNELMASK:
            if ( testProtocolHeader(4))  {
                fifo3Put('!'); 
                spiChecksum = 0;    
                for ( help = 0; help < 8; help++)  {
                   data =  getChannelMaskByte(spiData[1], spiData[2] * 8U + help);
                   spiChecksum ^= data;  
                   fifo3Put(data); 
                }
                fifo3Put(spiChecksum); 
            }
            break; 

        case I2CCMD_SETSCENECHANNELLEVEL:
             if ( testProtocolHeader(5))  {
                 storeSceneLevel(spiData[1], spiData[2], spiData[3]);
                 fifo3Put('!'); 
             }
             break;
        case I2CCMD_SETSCENECHANNELDIMSPEED:      
             if ( testProtocolHeader(5))  {
                 storeSceneDimSpeed(spiData[1], spiData[2], spiData[3]);
                 fifo3Put('!'); 
             }
             break;
        case I2CCMD_SETSCENECHANNELTRANSPARENCY :      
             if ( testProtocolHeader(5))  {
                 storeSceneTransparency(spiData[1], spiData[2], spiData[3]);
                 fifo3Put('!'); 
             }
             break;
            
        case I2CCMD_SETSCENECHANNELVALUE:
            
              if ( testProtocolHeader(7))  {
                    setSceneChannelLevel(spiData[1],spiData[2],spiData[3],spiData[4],spiData[5]);
                  fifo3Put('!'); 
             }
             break;
             
        case I2CCMD_GETSCENECHANNELVALUE :
             if ( testProtocolHeader(4))  {
                 fifo3Put('!'); 
                 data = getSceneChannelLevel(spiData[1], spiData[2]);
                 spiChecksum = data; 
                 fifo3Put(data);  
                 
                 data =  getSceneChannelTransparency(spiData[1], spiData[2]);
                 spiChecksum ^= data; 
                 fifo3Put(data);  
                 
                 data = getSceneChannelSpeed(spiData[1], spiData[2]);
                 spiChecksum ^= data; 
                 fifo3Put(data);  
                 fifo3Put(spiChecksum);
             }
             break; 

        case I2CCMD_GETMEMORYBLOCK:
             if ( testProtocolHeader(3))  {
                fifo3Put('!'); 
                bGetDataBlockActive = 1;
                dataBlockCounter    = 0; 
                dataBlockPage       = spiData[1]; 
             }
             break; 

        default:
            spiCount = 0; 
            break; 
    }
}

/*
 * Tests incoming data for good checksum
 * returns: 0 - false - in case of checksum mismatch
 *                    - in case checksum is not yet received
 *          1 - true  - if checksum fits.
 */

unsigned char testProtocolHeader(unsigned char numBytes)  {

    if ( spiCount == numBytes )  {   
        spiCount = 0; 
        
        if ( spiChecksum != 0)   {
            fifo3Put('?');
            return 0; 
        }
        else  {   
            //fifo3Put('!'); //Ne, nicht hier acknowledgen. Nach der payload ist es besser
            return 1; 
        }  
    }    
    return 0; 
   
}

/*
 *   Wenn Memorypages zum senden anstehen, wird das hier abgewickelt
 */
    
void i2cSendRawData()  {

    static unsigned char sumcheck  = 0;  
    
    unsigned char b; 
    
    if ( !bGetDataBlockActive )  
        return; 
    
    if( !fifo3Empty() )
       return;         
    
    b = getMemoryByte(dataBlockPage, dataBlockCounter);
   // b = dataBlockCounter; 
    fifo3Put(b);     
   
    sumcheck ^= b; 
    dataBlockCounter++;
    
    if ( dataBlockCounter == 0)    {
        fifo3Put(sumcheck ) ;    
        sumcheck = 0; 
        bGetDataBlockActive = 0; 
    }
    
}    


/*
 * Hardwareprotokoll adapter
 */

void i2cSlaveHandler(void)  {
    
    unsigned char data; 
    
    if ( SSP2STATbits.RW2 == 0 && 
         SSP2STATbits.DA2 == 0 && 
         SSP2STATbits.BF == 1)       {   // Master Write :: Address
         data = SSP2BUF;   // dummyread 
    }
    else if ( SSP2STATbits.RW2 == 0 && 
              SSP2STATbits.DA2 == 1 && 
              SSP2STATbits.BF == 1 )  {   // Master Write :: Data
         data = SSP2BUF;
         fifo2Put(data);
    }
    else if ( SSP2STATbits.RW2 == 1 && 
              SSP2STATbits.DA2 == 0   )  {   // Master Read :: Address
          SSP2BUF;   // Register anfassen.
          if ( fifo3Empty())  
              data = '*';
          else 
              data = fifo3Get();
                
          while(SSP2STATbits.BF)
             ;
          do {
             SSP2CON1bits.WCOL = 0; 
             SSP2BUF = data; 
          } 
          while (SSP2CON1bits.WCOL);
    }
    else if ( SSP2STATbits.RW2 == 1 && 
              SSP2STATbits.DA2 == 1 && 
              SSP2STATbits.BF == 0 )  {   // Master Read :: Data
               
          data = '_'; //nix.
     
          while(SSP2STATbits.BF)
             ;
          do { 
             SSP2CON1bits.WCOL = 0; 
             SSP2BUF = data; 
          } 
          while (SSP2CON1bits.WCOL);
    }
    else if ( SSP2STATbits.DA2 == 1 && 
              SSP2STATbits.BF == 0 && 
              SSP2CON1bits.CKP == 1)   {
               
               // hier irgendeinen reset machen.
               
    }
    SSP2CON1bits.CKP = 1;
}
