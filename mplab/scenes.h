/* 
 * File:   scenes.h
 * Author: marcus
 *
 * Created on 30. September 2014, 18:41
 */

#ifndef SCENES_H
#define	SCENES_H

/* 
 * Speicherabbild
 *  So ist eine Szene im I2C Flash abgelegt
 */

typedef struct _scenechannel  {

    unsigned char level;             
    unsigned char isTransparent ;   // nicht verändern, wenn 1
    unsigned char dimSpeed;         
} SCENECHANNEL;

/*
 *  Ablaufsteuerung Szenenaufruf
 */

#define MAXSCENES 128U

#define KEYPRESSTIME 4

typedef struct _scenecontrol  {
    
  unsigned char onCount : 6;   
    
  //unsigned char eventOn  : 1;    
  //unsigned char eventOff : 1;  
  unsigned char isOn     : 1;  
  //unsigned char continuousMode : 1; 
  unsigned char offPrepared    : 1; 
  
  
} SCENECONTROL;

/*
 *  Szenenapi nach "aussen"
 */
extern void callScene(unsigned char scene);
extern void doBlackout(unsigned char dim);
extern void setSceneLevel(unsigned char scene, unsigned char lavel ); 
extern unsigned char getCurrentScene(void);
extern void storeCurrentLevelsAsScene(unsigned char scene);
extern void storeChannelmask(unsigned char scene, unsigned char *maskArray);

/*
 * Szenenapi für die Aktoren
 */
extern void initSceneEvents(void); 

void initSceneChangeHardware(void);

extern void setEventOn (unsigned char scene, unsigned char mode);
extern void setEventOff(unsigned char scene, unsigned char mode);  
extern void handleSceneEvents(void); 

extern void handleSceneOn(unsigned char scene,  unsigned char mode);
extern void handleSceneOff(unsigned char scene, unsigned char mode);


extern void incrementSceneCounter(unsigned char scene);
extern void resetSceneCounter(unsigned char scene);



/*
 *  Persistence
 */

/*
 * Schnellere hilsfunktion; wird von der Dimengine verwendet
 */
unsigned char readSceneTransparency(unsigned char scene, unsigned char channel);


void readSceneChannel(unsigned char scene, unsigned char channel, SCENECHANNEL *scp);
void storeASceneChannel(unsigned char scene, unsigned char channel, SCENECHANNEL *scp );

void setSceneChannelLevel(unsigned char scene, unsigned char channel, unsigned char level, unsigned char transparency, unsigned char time);


unsigned char getSceneChannelLevel(unsigned char scene, unsigned char channel);
unsigned char getSceneChannelTransparency(unsigned char scene, unsigned char channel);
unsigned char getSceneChannelSpeed(unsigned char scene, unsigned char channel);

extern unsigned char getChannelMaskByte(unsigned char scene, unsigned char addr);

extern void storeSceneLevel(unsigned char scene, unsigned char channel, unsigned char level);
extern void storeSceneDimSpeed(unsigned char scene, unsigned char channel, unsigned char level);
extern void storeSceneTransparency(unsigned char scene, unsigned char channel, unsigned char transparency);

extern void didibag( unsigned char state);



#endif	/* SCENES_H */

