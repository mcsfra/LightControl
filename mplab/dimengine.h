#ifndef __DIMENGINE_H__
#define __DIMENGINE_H__

#define ENGINE_MAXCHANNEL 128U


#define STOP_IMMEDIATE  0
#define STOP_ATENDVALUE 1


typedef enum _ddimstate { DIMSTATE_IDLE = 0, 
                          DIMSTATE_UP = 1, 
                          DIMSTATE_REVERSEUP = 2, 
                          DIMSTATE_DOWN = 3, 
                          DIMSTATE_REVERSEDOWN = 4,
                          DIMSTATE_DELAYED_UP = 5
                        } DIMSTATE;

typedef struct _deChannel  {
    
    union  {
        unsigned int calc;
        struct {
           unsigned char fraction;
           unsigned char level;
        } level;
    } level;
    unsigned char   dimSteps; 
    signed int      increment; 
    //DIMSTATE      nextDimState : 3;
    unsigned char   delay        : 5;  
    unsigned char   nextDimState : 3;

    
} CHANNEL;


extern void initDimEngine(void); 
extern void dimEngineStep(void);
extern void initDimToTargetScene(unsigned char scene, DIMSTATE nextDimState);
extern void initDimToZero(unsigned char scene);
extern void prepareDim(unsigned char scene);
extern void dimStop(unsigned char scene, unsigned char mode);
extern void setDimEngineChannelLevel(unsigned char channel, unsigned char level);
extern void setChannelLevel(unsigned char channel, unsigned char level);
extern unsigned char getChannelLevel(unsigned char channel);

extern void calcAndSetIncrement(CHANNEL *cp, unsigned char level);

#endif
