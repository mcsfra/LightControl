
# 1 "fifo.c"

# 8 "fifo.h"
extern void fifoInit ( void );

extern unsigned char fifo1Get ( void );
extern void fifo1Put ( unsigned char in );
extern unsigned char fifo1Empty ( void );

extern unsigned char fifo2Get ( void );
extern void fifo2Put ( unsigned char in );
extern unsigned char fifo2Empty ( void );

extern unsigned char fifo3Get ( void );
extern void fifo3Put ( unsigned char in );
extern unsigned char fifo3Empty ( void );

# 12 "fifo.c"
unsigned char fifo1[16];
unsigned char fifo2[16];
unsigned char fifo3[16];

# 19
unsigned char fifo1In;
unsigned char fifo2In;
unsigned char fifo3In;

# 26
unsigned char fifo1Out;
unsigned char fifo2Out;
unsigned char fifo3Out;

# 34
void fifoInit() {

unsigned char i;

for ( i = 0; i < 16; i++) {
fifo1[i] = 0x00;
fifo2[i] = 0x00;
fifo3[i] = 0x00;
}

fifo1In = 0;
fifo1Out = 0;
fifo2In = 0;
fifo2Out = 0;
fifo3In = 0;
fifo3Out = 0;
}

# 58
unsigned char fifo1Get() {
return fifo1[fifo1Out++ & 0x0FU];
}

unsigned char fifo2Get() {
return fifo2[fifo2Out++ & 0x0FU];
}

unsigned char fifo3Get() {
return fifo3[fifo3Out++ & 0x0FU];
}

# 76
void fifo1Put(unsigned char in) {
fifo1[fifo1In++ & 0x0FU] = in;
}

void fifo2Put(unsigned char in) {
fifo2[fifo2In++ & 0x0FU] = in;
}

void fifo3Put(unsigned char in) {
fifo3[fifo3In++ & 0x0FU] = in;
}

# 93
unsigned char fifo1Empty() {

return (fifo1In & 0x0FU) == (fifo1Out & 0x0FU);

}

unsigned char fifo2Empty() {

return (fifo2In & 0x0FU) == (fifo2Out & 0x0FU);

}


unsigned char fifo3Empty() {

return (fifo3In & 0x0FU) == (fifo3Out & 0x0FU);

}

