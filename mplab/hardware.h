#ifndef __HARDWARE_H__
#define	__HARDWARE_H__


// Pinning
//
//  1 -- MCLR
// 11 -- Vdd
// 12 -- Vss

// 25 -- TX1  -- DMX Out

// 30 -- RX2 -- Tastenencoder Eingang

// 31 -- Vss
// 32 -- Vdd

// 39 -- RB6 -
// 40 -- RB7 -

/*
 *    I2C kommunikation mit meinem "grossen" master
 */



//





#define TRISAMASK   0b00000111


#define TRISBMASK   0b00100111

//  -- RC0
//  -- RC1
//  -- RC2
//  -- RC3
//  -- RC4
//  -- RC5
//  -- RC6
//  -- RC7  -- TX1 



#define TRISCMASK   0b10111111


//  -- RD0
//  -- RD1
//  -- RD2
//  -- RD3
//  -- RD4
//  -- RD5
//  -- RD6
//  -- RD7  -- RX2



#define TRISDMASK   0b11111111




extern void initHardware(void);





#endif	/* HARDWARE_H */

