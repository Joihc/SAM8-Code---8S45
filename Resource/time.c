#define _TIME_C_

#include "time.h"
/*
  8MHZ 1us  6*X+5us   太多了就要另外算 只能大概
*/
void delay(volatile uint16 ms)
{
    ms=ms*12+10;//140
    do{
      asm("nop"); ms--;
    }while(ms!=0);
}
void delayus(volatile uint8 us)
{

   do{
     nop; us--;
   }while(us!=0);
}
