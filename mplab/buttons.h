/* 
 *  buttons.h - McS - 2014 
 */

#ifndef BUTTONS_H
#define	BUTTONS_H


typedef unsigned char BUTTONINDEX;      // use in loops
typedef unsigned char BUTTONNUMBER;     // incoming from protocol
typedef unsigned char ASSIGNEDSCENE;    // internal meaning


typedef struct button  {

   BUTTONNUMBER   buttonNumber;      // device und nummer des Knopfs
   ASSIGNEDSCENE  assignedScene; 
   
   unsigned char buttonMode : 1;      // 0 - wie gehabt; 1 - continuous dim
   unsigned char isActive   : 1;      // ist der knopf akuell als aktiv erkannt ?
   unsigned char watchdog   : 4;      // zähler. Ich Idiot: Ich hatte nur ein Bit für nen ZÄHLER vorgesehe. 
   
} BUTTON;



extern void initButtonHardware(void);
extern void decodeButton(unsigned char code);
extern void buttonsReset();
extern void initButton(BUTTONINDEX i, BUTTONNUMBER bn, ASSIGNEDSCENE as, unsigned char buttonMode ) ;
extern void initAllButtons(void);
extern void testRC2Errors(void);
extern void registerButton(unsigned char button, unsigned char action, unsigned char buttonMode);
/*
 *  Anbindung "Massenspeicher"
 */

extern void storeAButton(unsigned char button, unsigned char program, unsigned char mode);
extern void readAButton(unsigned char button, unsigned char *data);
extern unsigned char getLastPressedKey(void);

#endif	/* BUTTONS_H */

