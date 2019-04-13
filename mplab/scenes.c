#include <xc.h>
#include "globaldefs.h"
#include "dimengine.h"
#include "scenes.h"
#include "massStorage.h"


/*
 * Current Scene
 */
unsigned char currentScene;



SCENECONTROL sceneControl[MAXSCENE]; 

/*
 * Init my own datastructures
 */

void initSceneEvents(void)   {
    
    unsigned char i = 0; 
    SCENECONTROL *scp = sceneControl; 
    
    for ( ; i < MAXSCENES; i++, scp++)  {
         scp -> isOn         = 0; 
         scp -> offPrepared  = 0;
         scp -> onCount      = 0;
    }
} 

/*
 * Will be called if a Key is pressed (Flank)
 */
void handleSceneOn(unsigned char scene, unsigned char mode)  {
    
    SCENECONTROL *scp = &sceneControl[scene];
    
    currentScene = scene;
    
    if ( mode )  {
        
        scp -> onCount = 0;    
        if ( scp -> isOn == 0)  {                         // if this scene is off
            scp -> isOn        = 1;     
            scp -> offPrepared = 0;
            
            initDimToTargetScene(scene, DIMSTATE_UP);     // 
        }
        else  {                             // if scene is on...
           scp -> offPrepared = 1;          // prepare to switch off...                     
           prepareDim(scene);               // ...and prepare to dim up if button is kept pressed.
        }
    }
    else  {
        initDimToTargetScene(scene, DIMSTATE_IDLE);
    }
}
    
/*
 * Called when Button is released.
 */

void handleSceneOff(unsigned char scene, unsigned char mode)  {
         
    SCENECONTROL *scp = &sceneControl[scene];
    
    if ( mode )  {
        
        if ( scp -> offPrepared )  {                       // 2. Tatendruck. Entweder Aus, wenn kurz oder Dimmen beenden, wenn lang   
             
            if ( scp -> onCount ==  KEYPRESSTIME)   {      // Langer tastendruck
                dimStop(scene,STOP_IMMEDIATE);            // Stop. Sofort! 
            }
            else  {                                        // kurzer Druck. 
                scp -> isOn = 0;                          // also Aus
                initDimToZero(scene);                     // und runterdimmen
            }
            scp -> offPrepared = 0; 
        }
        else {
                
            if ( scp -> onCount ==  KEYPRESSTIME)   {
                dimStop(scene,STOP_IMMEDIATE);      // Langer Impuls beim EINSCHALTEN 
            }
            else  {
                dimStop(scene,STOP_ATENDVALUE);     // Kurzes Einschalten. Auf Endwert dimmen. 
            }
        }
    }
}

/* 
 *  handleSceneEvents
 * 
 *  Steuert den dimmechanismus für beide Modes:
 *  
 *  ** Szene anfahren
 *  ** Kontinuierliches dimmen
 * 
 *  Aufruf aus der hauptschleife. Diese Methode vermittelt zwischen eingehenden Button-events und 
 *  der Dimm-Engine. 
 */


void incrementSceneCounter(unsigned char scene)  {
    
        if ( sceneControl[scene].onCount < KEYPRESSTIME)  { 
             sceneControl[scene].onCount = sceneControl[scene].onCount + 1 ; 
             // ^^ Warning? Why?
        }   
}


/*
 *  Gibt die aktuelle Szenennummer zurück
 */

unsigned char getCurrentScene(void)  {

    return currentScene;
}

/*
 *  Setzt die Kanaele einer szene auf euinen Wert
 *  Die "grosse" versicon soll da sgewichtet tun.
 **/


void setSceneLevel(unsigned char scene, unsigned char level )  {
    
    unsigned char i; 
    SCENECHANNEL sc;
    
    for ( i = 0; i < MAXCHANNEL; i++) {
   
        readSceneChannel(scene, i, &sc);
        
        if ( sc.isTransparent == 0 && sc.level > 0 )  {
            setChannelLevel(i, level); 
        } 
     } 
} 



/*
 * Berechnet die Adresse eines Kanals innerhalb einer Szene
 */

unsigned int sceneAdress(unsigned char scene, unsigned char channel)  {

    unsigned int baseAdress = SCENE_MEMORY_START;
    baseAdress += ((scene-1U) * sizeof(SCENECHANNEL) * MAXCHANNEL) + (channel * sizeof(SCENECHANNEL));
    return baseAdress;
}

/*
 * Liest eine Kanalinformation aus einer Szene
 */

void readSceneChannel(unsigned char scene, unsigned char channel, SCENECHANNEL *scp)  {

    unsigned int baseAdress = sceneAdress(scene, channel);

    HDByteReadI2C( 0xA0, (unsigned char )(baseAdress >> 8U) , (unsigned char )(baseAdress & 0xFFU) ,(unsigned char*) scp, sizeof(SCENECHANNEL) );
}

unsigned char getSceneChannelLevel(unsigned char scene, unsigned char channel)  {

    SCENECHANNEL sc;
    readSceneChannel(scene, channel, &sc);
    return sc.level;
}
unsigned char getSceneChannelTransparency(unsigned char scene, unsigned char channel)  {

    SCENECHANNEL sc;
    readSceneChannel(scene, channel, &sc);
    return sc.isTransparent;
}

unsigned char getSceneChannelSpeed(unsigned char scene, unsigned char channel)  {

    SCENECHANNEL sc;
    readSceneChannel(scene, channel, &sc);
    return sc.dimSpeed;
}
 

unsigned char readSceneTransparency(unsigned char scene, unsigned char channel)  {
    
    unsigned char transparency; 
    unsigned int baseAdress = sceneAdress(scene, channel);

    HDByteReadI2C( 0xA0, (unsigned char )(baseAdress >> 8U) , (unsigned char )(baseAdress & 0xFFU) + 1U , &transparency, 1 );

    return transparency;
}




void setSceneChannelLevel(unsigned char scene, unsigned char channel, unsigned char level, unsigned char transparency, unsigned char time)  {

    SCENECHANNEL sc;

    sc.level         = level;
    sc.dimSpeed      = time;
    sc.isTransparent = transparency;

    storeASceneChannel(scene, channel, &sc);
}


/*
 *  Timer 3
 */

void initSceneChangeHardware()  {
    
    T3CON = 0;

    /*
     * Rev 1.1 - Prescaler auf 1:1. war 1:2 (1)
     */

    T3CONbits.T3CKPS = 0;
    T3CONbits.TMR3ON = 1;

    IPR2bits.TMR3IP = 0;
    PIE2bits.TMR3IE = 1;
}


void storeASceneChannel(unsigned char scene, unsigned char channel, SCENECHANNEL *scp )  {

    unsigned int baseAdress = sceneAdress(scene, channel);

    HDByteWriteI2C( 0xA0, (unsigned char )(baseAdress >> 8) , (unsigned char )(baseAdress & 0xFFU) ,   scp -> level );
    HDByteWriteI2C( 0xA0, (unsigned char )(baseAdress >> 8) , (unsigned char )(baseAdress & 0xFFU)+1U , scp -> isTransparent);
    HDByteWriteI2C( 0xA0, (unsigned char )(baseAdress >> 8) , (unsigned char )(baseAdress & 0xFFU)+2U , scp -> dimSpeed);
}

/*
 * Blackout: Alles runterdimmen. 
 *  dim = 1 --> Dimmen
 *  dim = 0 --> Alles runter auf null. Sofort!
 */

void doBlackout(unsigned char dim)  {
    
    unsigned char i; 
    for ( i = 0; i < MAXCHANNEL; i++) {
       setChannelLevel(i,0);
    }
}

void callScene(unsigned char scene)  {
    
    currentScene = scene; 
    initDimToTargetScene(scene, DIMSTATE_IDLE);
   
}
 
/*
 *  Speichert die Kanalmaske für eine Szene 
 */

void storeChannelmask(unsigned char scene, unsigned char *maskArray)  {
    
   unsigned char i; 
   unsigned int baseAddress; 
   unsigned char val;
   
   unsigned char control; 
   
   for ( i = 0; i < MAXCHANNEL; i++)  {
       baseAddress = sceneAdress(scene, i);
       
       val = (maskArray[i/8] & (1<<(7-(i%8)))) != 0 ? 0 : 1U; 
       HDByteWriteI2C( 0xA0U, (unsigned char )(baseAddress >> 8) , (unsigned char )(baseAddress & 0xFFU) + 1U , val );
       
       /*
        * Kontrolllesen. Ich glaube ncith, dass ich hier ien issue habe, jkann mir das verhalten des 
        * getChannelMask testcases aber nicht anders erklaeren.
        */
       
       HDByteReadI2C( 0xA0U, (unsigned char )(baseAddress >> 8) , (unsigned char )(baseAddress & 0xFFU) + 1U ,&control, 1 );
       
       if ( val != control)  {
           HDByteWriteI2C( 0xA0U, (unsigned char )(baseAddress >> 8) , (unsigned char )(baseAddress & 0xFFU) + 1U , val );
       }
   } 
}

/*
 *  "komprimiert" die Maskenbits aus einer szene in ein byte
 */

unsigned char getChannelMaskByte(unsigned char scene, unsigned char addr) {
    
   unsigned char i;  
   unsigned char r = 0;  
   unsigned char t = 0; 
   
   for ( i = addr * 8U; i < (addr+1U) * 8U; i++)  {
      
       t = getSceneChannelTransparency(scene, i); 
       
       if ( t == 0 )  {
           r |= 1U << (7U-(i - addr*8U)); 
       }
   }
   return r; 
}




/*
 * Speichert das Aktuell eingestellte Kanalpegel als Szene
 */

void storeCurrentLevelsAsScene(unsigned char scene)  {
   
   unsigned char i; 
   unsigned int  baseAddress;  
   
   for ( i = 0; i < MAXCHANNEL; i++)  {
       baseAddress = sceneAdress(scene, i);
       HDByteWriteI2C( 0xA0U, (unsigned char )(baseAddress >> 8) , (unsigned char )(baseAddress & 0xFFU) , getChannelLevel(i) );

   }    
}

/*
 * Stores a Channellevel for a scene
 */
void storeSceneLevel(unsigned char scene, unsigned char channel, unsigned char level) {

    unsigned int baseAddress = sceneAdress(scene, channel);
    HDByteWriteI2C( 0xA0U, (unsigned char )(baseAddress >> 8) , (unsigned char )(baseAddress & 0xFFU) , level );
}


/*
 * Stores a Channel DimSpeed for a scene
 */
void storeSceneDimSpeed(unsigned char scene, unsigned char channel, unsigned char dimSpeed)  {
    
    unsigned int baseAddress = sceneAdress(scene, channel);
    HDByteWriteI2C( 0xA0U, (unsigned char )(baseAddress >> 8U) , (unsigned char )(baseAddress & 0xFFU) + 2U , dimSpeed );
    
}

void storeSceneTransparency(unsigned char scene, unsigned char channel, unsigned char transparency) {

    unsigned int baseAddress = sceneAdress(scene, channel);
    HDByteWriteI2C( 0xA0U, (unsigned char )(baseAddress >> 8U) , (unsigned char )(baseAddress & 0xFFU) + 1U , transparency );
}

