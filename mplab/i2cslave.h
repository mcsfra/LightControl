/* 
 * File:   i2cslave.h
 * Author: marcus
 *
 * Created on 27. Oktober 2014, 19:37
 */

#ifndef I2CSLAVE_H
#define	I2CSLAVE_H

/*
 * Das möchte ich gerne im interrupt haben und nicht das zeug was ich da aktuell habe,
 */
extern void i2cSlaveHandler(void);


extern void initI2CSlave(void);

extern void initI2CParser();
extern void i2cParser(unsigned char data);

extern void i2cSendRawData(void);   

#endif	/* I2CSLAVE_H */

