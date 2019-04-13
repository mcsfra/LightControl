#ifndef I2C_H
#define I2C_H


extern void i2cInit(void);
extern void i2cStart(void);
extern void i2cStop(void);
extern void i2cRestart(void);
extern void i2cAck(void);
extern void i2cNak(void);
extern void i2cWait(void);
extern void i2cSend(unsigned char dat);
extern signed char i2cWrite(unsigned char dat);
extern void i2cIdle(void);

extern unsigned char i2cRead(void);  

extern signed char EEAckPolling( unsigned char control );
extern signed char i2cGets( unsigned char *rdptr, unsigned char length );
    
#endif