#include <xc.h>
#include "globaldefs.h"
#include "dimengine.h"
#include "scenes.h"
//#include "debug.h"

/*
 *  Kanaldaten
 */
CHANNEL channels[MAXCHANNEL];

/*
 *  Initialisieren
 */

void initDimEngine(void)  {
    
    unsigned char i; 
    
    for (i = 0; i < ENGINE_MAXCHANNEL; i++ /*, cp++ */)  {
   
        channels[i].dimSteps     = 0;
        channels[i].increment    = 0;
        channels[i].nextDimState = DIMSTATE_IDLE;
        channels[i].level.calc   = 0;
        channels[i].delay        = 0;
    }
}

/*
 * dimEngineStap 
 * called by timer to ensure isochron execution
 * 
 *  will either change channel value or switch to next dim-state for continiouos up/down dimming
 */

#define CM_STEPS 64U

void dimEngineStep(void)  {
    
  unsigned char i; 
  
  CHANNEL *cp = channels; 
  
  for (i = 0; i < ENGINE_MAXCHANNEL; i++, cp++)  {
      
      if ( cp -> dimSteps > 0)  {
          INTCONbits.GIEH = 0; 
          cp -> level.calc += cp -> increment; 
          INTCONbits.GIEH = 1; 
          cp -> dimSteps--; 
      }
      else   {
        if ( cp -> nextDimState != DIMSTATE_IDLE /*&& cp -> increment != 0 */)    {
            
            cp -> dimSteps = CM_STEPS;      
            
            switch (cp -> nextDimState )  { 
                    
               case DIMSTATE_UP :
                    calcAndSetIncrement(cp, 255);
                    cp -> nextDimState = DIMSTATE_REVERSEUP; 
                    break; 
                    
               case DIMSTATE_REVERSEUP :
                    cp -> level.level.fraction = 0x00; 
                    cp -> increment = - cp -> increment; 
                    cp -> nextDimState = DIMSTATE_DOWN; 
                    break; 
                    
               case DIMSTATE_DOWN :
                    calcAndSetIncrement(cp, 0);
                    cp -> nextDimState = DIMSTATE_REVERSEDOWN; 
                    break; 
                    
               case DIMSTATE_REVERSEDOWN :
                    cp -> level.level.fraction = 0x00; 
                    cp -> increment = - cp -> increment; 
                    cp -> nextDimState = DIMSTATE_UP; 
                    break; 
                    
               case DIMSTATE_DELAYED_UP :
                     // zum test
                    cp -> nextDimState = DIMSTATE_UP; 
                   
                    // Puuh: Tatsächlich funktioniert es ohne "delay" much besser.
                    // Aber die statemachine funktioniert nicht.
                    // es dimmt sich runter nach dem loslassen.
                    // Seufz, das sollte ja inzwischem gehen....
                    
                     // Ha! Hier in diesem state scheint es paasieren; 
                    if ( cp -> delay >= 20 )
                      cp -> nextDimState = DIMSTATE_UP; 
                    else 
                      cp -> delay++;    
                    break; 
                    
            }  // switch 
        }
    }
  }
}

/*
 * Dimmen initialisieren auf den endwert
 */

void initDimToTargetScene(unsigned char scene, DIMSTATE nextDimState)  {
    
    SCENECHANNEL sc;
    unsigned char i;
    
    CHANNEL *cp = channels; 
  
    for (i = 0; i < ENGINE_MAXCHANNEL; i++, cp++)  {
   
        readSceneChannel(scene, i, &sc);
        if ( sc.isTransparent != 0  )  
            continue; 
        
        cp -> nextDimState = nextDimState;      
        cp -> level.level.fraction = 0x00; 
        cp -> delay = 0;
        /*
         * Dieser wert soll in zwei varianten gesetzt werden: 
         * 1. für Szenenanfahren soll der Wert aus der Szene kommen, 
         * 2. Für kontinuierliches Dimmen soll die Dimmzeit für alle betroffenen kanäle 
         *    Identisch sein, ich brauche hier also  einen override. 
         *    eventuell kann ich das über den Dimstate hängen.
         */
        if ( nextDimState == DIMSTATE_IDLE )
           cp -> dimSteps = sc.dimSpeed == 0 ? CM_STEPS : sc.dimSpeed;  // die 896 stehen hier zu demo zwecken. es sollte 1 sein, oder? 
        else  
           cp -> dimSteps = CM_STEPS;
       
        calcAndSetIncrement(cp, sc.level);
    }
     
}

void prepareDim(unsigned char scene)  {
    
    unsigned char i;
    
    CHANNEL *cp = channels; 
  
    for (i = 0; i < ENGINE_MAXCHANNEL; i++, cp++)  {
    
       if (readSceneTransparency(scene, i))  // Schnellere Variante
          continue; 
        
        cp -> nextDimState = DIMSTATE_DELAYED_UP;    
        cp -> level.level.fraction = 0x00; 
        cp -> delay    = 0; 
        cp -> dimSteps = 0; 
    }
}



void initDimToZero(unsigned char scene)  {
    
    unsigned char i;

    CHANNEL *cp = channels; 
  
    for (i = 0; i < MAXCHANNEL; i++, cp++)  {
    
       if (readSceneTransparency(scene, i))
          continue; 
        
        cp -> level.level.fraction = 0x00; 
        
        cp -> nextDimState = DIMSTATE_IDLE; 
        cp -> dimSteps     = CM_STEPS;
         
        calcAndSetIncrement(cp, 0);
    }
     
}

void dimStop(unsigned char scene, unsigned char mode)  {
    
  unsigned char i; 
  SCENECHANNEL sc;
  CHANNEL *cp = channels; 
  
  for (i = 0; i < MAXCHANNEL; i++, cp++)  {
       
       if ( mode == STOP_IMMEDIATE )  {
          cp -> dimSteps   = 0;
          cp -> increment  = 0; 
          cp -> level.level.fraction = 0x00; 
       } 
     
       cp -> nextDimState = DIMSTATE_IDLE;
  }
}

/*
 * Berechnet den Incrementwert basierend auf dem Aktuellen Kanalwert, 
 * dem Endwert und der Anzahl schritte. 
 * Berechnung mit integers war nicht präzise genug, deswegen float.
 */


void calcAndSetIncrement(CHANNEL *cp, unsigned char targetLevel)  {
    
    float help; 
    
    help = (float) ((int)targetLevel - (int)cp -> level.level.level);
    help *= 256.0;
    help /= (float) cp -> dimSteps ; 
        
    cp -> increment = (int)help; 
}

/*
unsigned char getDimEngineChannelLevel(unsigned char channel)  {
    
    return channel < MAXCHANNEL ? channels[channel].level.level.level : 0;
}
*/

unsigned char getChannelLevel(unsigned char channel)  {
    
    return channel < MAXCHANNEL ? channels[channel].level.level.level : 0;
}


void setChannelLevel(unsigned char channel, unsigned char level)  {

    if ( channel < MAXCHANNEL)
       channels[channel].level.level.level = level;
}


// EOF