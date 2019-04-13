#include <xc.h>
#include "i2cmemory.h"


unsigned char bI2CTimeout;


void i2cInit(void)  {
 
    TRISCbits.RC3 = 1; 
    TRISCbits.RC4 = 1; 
    
    SSPCON1bits.SSPEN = 1;
    SSPADD  = 0x08;   // das stimmt doch nicht, oder? 
    SSPCON1 = 0x28;
    SSPSTAT = 0x00;
    
    bI2CTimeout = 0; 
}

/*
 * Von Microchip gemopst
 */

/*
Function: I2CStart
Return:
Arguments:
Description: Send a start condition on I2C Bus
*/
void i2cStart()  {

   SSPCON2bits.SEN = 1;   // Start condition enabled 
   while(SSPCON2bits.SEN) // automatically cleared by hardware 
       ;
}

/*
Function: I2CStop
Return:
Arguments:
Description: Send a stop condition on I2C Bus
*/
void i2cStop()   {
  
   SSPCON2bits.PEN = 1;  // Stop condition enabled 
   while(SSPCON2bits.PEN) // Wait for stop condition to finish 
       ;
}

/*
Function: I2CRestart
Return:
Arguments:
Description: Sends a repeated start condition on I2C Bus
*/
void i2cRestart()   {

   SSPCON2bits.RSEN = 1; /* Repeated start enabled */
   while(SSPCON2bits.RSEN) /* wait for condition to finish */
      ;
}

/*
Function: I2CAck
Return:
Arguments:
Description: Generates acknowledge for a transfer
*/
#ifdef UNUSED_FUNCTION
void i2cAck()  {

   unsigned int timeout = 0xFFFF; 
    
   SSPCON2bits.ACKDT = 0; /* Acknowledge data bit, 0 = ACK */
   SSPCON2bits.ACKEN = 1; /* Ack data enabled */
   while(SSPCON2bits.ACKEN)   { /* wait for ack data to send on bus */
      if ( timeout-- == 0)  {
          bI2CTimeout = 1; 
          return; 
      }
   }
}
#endif

/*
Function: I2CNck
Return:
Arguments:
Description: Generates Not-acknowledge for a transfer
*/
void i2cNak()  {
    
   unsigned int timeout = 0xFFFFU;  
    
   SSPCON2bits.ACKDT = 1; /* Acknowledge data bit, 1 = NAK */
   SSPCON2bits.ACKEN = 1; /* Ack data enabled */
  
   while(SSPCON2bits.ACKEN)   { /* wait for ack data to send on bus */
      if ( timeout-- == 0)  {
          bI2CTimeout = 1;
          return; 
      }
   }
}

/*
Function: I2CWait
Return:
Arguments:
Description: wait for transfer to finish
*/
void i2cWait()  {

     unsigned int timeout = 0xFFFFU; 
    
     while ((SSPCON2 == 0x1F ) || ( SSPSTAT == 0x04 ) )  {
         if ( timeout-- == 0 )  {
            bI2CTimeout = 1; 
            return;
         }   
     }
}

/*
Function: I2CSend
Return:
Arguments: dat - 8-bit data to be sent on bus
data can be either address/data byte
Description: Send 8-bit data on I2C bus
*/
signed char i2cWrite(unsigned char dat)  {

   SSPBUF = dat;    // Move data to SSPBUF
   
   if ( SSPCON1bits.WCOL)
       return -1; 
   
   while(SSP1STATbits.BF)        // wait till complete data is sent from buffer 
      ; 
   i2cIdle(); 
   
   if( SSPCON2bits.ACKSTAT)
       return -2; 
   
   return 0; 
}

#ifdef UNUSED_FUNCTION
void i2cSend(unsigned char dat)  {

   SSPBUF = dat;    // Move data to SSPBUF 
   while(SSP1STATbits.BF)        // wait till complete data is sent from buffer 
      ; 
   i2cWait(); 
}
#endif
/*
Function: I2CRead
Return: 8-bit data read from I2C bus
Arguments:
Description: read 8-bit data from I2C bus
*/
unsigned char i2cRead(void)  {

    unsigned int timeout = 0xFFFF; 
    
    unsigned char temp;
                                // Reception works if transfer is initiated in read mode 
    SSPCON2bits.RCEN = 1;       //  Enable data reception 
    while(!SSP1STATbits.BF)  {   // wait for buffer full 
     
        if ( timeout-- == 0)  {
            bI2CTimeout = 1;
            return 0; 
        }
    }
    
    temp = SSPBUF;   /* Read serial buffer and store in temp register */
    i2cWait();       /* wait to check any pending transfer */
    return temp;     /* Return the read data from bus */
}

void i2cIdle(void)  {
   
     while ( ( SSPCON2 & 0x1F ) || ( SSPSTATbits.R_W ) )
        ;
}

signed char EEAckPolling( unsigned char control )  {
    
  i2cIdle();                      // ensure module is idle 
  i2cStart();                     // initiate START condition
  while ( SSPCON2bits.SEN )      // wait until start condition is over 
      ;
  if ( PIR2bits.BCLIF )  {            // test for bus collision
    return -1;                // return with Bus Collision error 
  }
  else   {
     if( i2cWrite( control ) == -1 )  {
         i2cStop();
         return -3;
     }

     while ( SSPCON2bits.ACKSTAT )   {  // test for ACK condition received
    
         i2cRestart();               // initiate Restart condition
         while ( SSPCON2bits.RSEN ) // wait until re-start condition is over 
            ;
         if ( PIR2bits.BCLIF )  {       // test for bus collision
            return  -1 ;            // return with Bus Collision error 
         }
      
         if ( i2cWrite( control ) == -1)  {    // Alternative fuer unteres statement
             i2cStop(); 
             return -3;
          
         }    
    }
  }
         
  i2cStop();                      // send STOP condition
  while ( SSPCON2bits.PEN )       // wait until stop condition is over         
     ;
  if ( PIR2bits.BCLIF )  {           // test for bus collision
    return -1;                // return with Bus Collision error 
  }
  return 0;                   // return with no error     
}



signed char i2cGets( unsigned char *rdptr, unsigned char length )  {
    
    while ( length-- )  {           // perform getcI2C() for 'length' number of bytes
    
      *rdptr++ = i2cRead();       // save byte received
      while ( SSPCON2bits.RCEN ) // check that receive sequence is over    
        ;
      if ( PIR2bits.BCLIF )  {       // test for bus collision
        return  -1 ;            // return with Bus Collision error 
      }

  	  if( ((SSPCON1&0x0F)==0x08) || ((SSPCON1&0x0F)==0x0B) )   {	//master mode only
		
          if ( length )   {               // test if 'length' bytes have been read
             SSPCON2bits.ACKDT = 0;    // set acknowledge bit state for ACK        
             SSPCON2bits.ACKEN = 1;    // initiate bus acknowledge sequence
             while ( SSPCON2bits.ACKEN )
                 ; // wait until ACK sequence is over 
          } 
	  } 
	  
    }
    return 0;                 // last byte received so don't send ACK      
}



