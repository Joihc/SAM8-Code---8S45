#ifndef _74HC164_H_
#define _74HC164_H_

#include "config.h"
#include "time.h"

#ifndef _74HC164_C_

#endif

enum LED_STATE
{
  OFF,
  ON,
  FLUSH
};


#define DAT_1 (Set_Bit(P2,5))
#define DAT_0 (Clr_Bit(P2,5))

#define CLK_1 (Set_Bit(P2,4))
#define CLK_0 (Clr_Bit(P2,4))

//红灯显示
#define COM_1_1 (Set_Bit(P2,2))
#define COM_1_0 (Clr_Bit(P2,2))
#define COM_COM_1 (Com_Bit(P2,2))

//数字显示开关
#define COM_2_1 (Set_Bit(P2,3))
#define COM_2_0 (Clr_Bit(P2,3))


void setNum_74HC164(uint8 num);
void init_74HC164();
void whileUpdate_74HC164();
void interuptUpdate_74HC164();

static void display();

#endif /* _74HC164_H_ */
