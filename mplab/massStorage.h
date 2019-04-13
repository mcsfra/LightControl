/* 
 * File:   massStorage.h
 * Author: marcus
 *
 * Created on 30. September 2014, 18:49
 */

#ifndef MASSSTORAGE_H
#define	MASSSTORAGE_H

//#define DEVICE_MEMORY_START 0x100;
#define SCENE_MEMORY_START  0x200;

void initI2C(void);



extern unsigned char getMemoryByte(unsigned char msb, unsigned char lsb);

extern void writeFlashByte(unsigned int address, unsigned char data);
extern void readFlash(unsigned int address, unsigned char *data, unsigned char length);


/*
 *   Test Stuff
 */

extern void writeTestPattern(unsigned char bank);
extern void readTestPattern(unsigned char bank);


/*
 *  Lowlevel stuff
 */

unsigned char HDByteWriteI2C( unsigned char ControlByte, unsigned char HighAdd, unsigned char LowAdd, unsigned char data );
unsigned char HDByteReadI2C(  unsigned char ControlByte, unsigned char HighAdd, unsigned char LowAdd, unsigned char *data, unsigned char length );
unsigned char LDByteReadI2C(  unsigned char ControlByte, unsigned char LowAdd, unsigned char *data, unsigned char length );
unsigned char LDByteWriteI2C( unsigned char ControlByte, unsigned char LowAdd, unsigned char data );




#endif	/* MASSSTORAGE_H */

