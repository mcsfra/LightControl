#ifndef __DEBUG_H__
#define __DEBUG_H__

#define MAXLOGBUFF 16

#define CHANNEL0 LATAbits.LATA0
#define CHANNEL1 LATAbits.LATA1
#define CHANNEL2 LATAbits.LATA2

void debugInit(void); 

extern void ping0(unsigned char numPings);
extern void ping1(unsigned char numPings);
extern void ping2(unsigned char numPings);

extern void toggle0(void);
extern void toggle1(void);
extern void toggle2(void);

extern void set0(void);
extern void set1(void);
extern void set2(void);

extern void reset0(void);
extern void reset1(void);
extern void reset2(void);

#endif

