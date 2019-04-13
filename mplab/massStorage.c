#include <xc.h>
#include "globaldefs.h"
#include "i2cmemory.h"
#include "massStorage.h"

/*
 *  Memory connected to I2C Port number 1
*/


/*
 * Initiale Funktionsprüfung des Massenspeichers
 */

unsigned char getMemoryByte(unsigned char msb, unsigned char lsb)  {

    unsigned char b[1];
    HDByteReadI2C( 0xA0,msb, lsb, b, 1 );
    return b[0];
}




void initI2C() {

    i2cInit(); 
   //OpenI2C(MASTER, SLEW_OFF);
   
   SSPADD = 119; // 0x18; //108; //49; //0x18;

}

/*
** Lowlevel Functions
*/




unsigned char HDByteWriteI2C( unsigned char ControlByte, unsigned char HighAdd, unsigned char LowAdd, unsigned char data )  {
    
  i2cIdle();                      // ensure module is idle
  i2cStart();                     // initiate START condition
  while ( SSPCON2bits.SEN )
      ;      // wait until start condition is over
  i2cWrite( ControlByte );        // write 1 byte - R/W bit should be 0
  i2cIdle();                      // ensure module is idle
  i2cWrite( HighAdd );            // write address byte to EEPROM
  i2cIdle();                      // ensure module is idle
  i2cWrite( LowAdd );             // write address byte to EEPROM
  i2cIdle();                      // ensure module is idle
  i2cWrite ( data );              // Write data byte to EEPROM
  i2cIdle();                      // ensure module is idle
  i2cStop();                      // send STOP condition
  while ( SSPCON2bits.PEN )
       ;      // wait until stop condition is over
  while (EEAckPolling(ControlByte)) // Hier hängsts
     ;  //Wait for write cycle to complete
 
  return  0;                   // return with no error
}








/********************************************************************
*     Function Name:    HDByteReadI2C                               *
*     Parameters:       EE memory ControlByte, address, pointer and *
*                       length bytes.                               *
*     Description:      Reads data string from I2C EE memory        *
*                       device. This routine can be used for any I2C*
*                       EE memory device, which only uses 1 byte of *
*                       address data as in the 24LC01B/02B/04B/08B. *
*                                                                   *
********************************************************************/

unsigned char HDByteReadI2C( unsigned char ControlByte, unsigned char HighAdd, unsigned char LowAdd, unsigned char *data, unsigned char length )
{
  i2cIdle();                      // ensure module is idle
  i2cStart();                     // initiate START condition
  while ( SSPCON2bits.SEN )
      ;      // wait until start condition is over
  i2cWrite( ControlByte );        // write 1 byte
  i2cIdle();                      // ensure module is idle
  i2cWrite( HighAdd );            // WRITE word address to EEPROM
  i2cIdle();                      // ensure module is idle
  while ( SSPCON2bits.RSEN )
      ;     // wait until re-start condition is over
  i2cWrite( LowAdd );             // WRITE word address to EEPROM
  i2cIdle();                      // ensure module is idle
  i2cRestart();                   // generate I2C bus restart condition
  while ( SSPCON2bits.RSEN )
      ;     // wait until re-start condition is over
  i2cWrite( ControlByte | 0x01U ); // WRITE 1 byte - R/W bit should be 1 for read
  i2cIdle();                      // ensure module is idle
  i2cGets( data, length );       // read in multiple bytes

  i2cNak();                    // send not ACK condition
 // schon durch
  //while ( SSPCON2bits.ACKEN )
 //      ;    // wait until ACK sequence is over
  i2cStop();                      // send STOP condition
  while ( SSPCON2bits.PEN )
      ;      // wait until stop condition is over
  return 0;                   // return with no error
}
