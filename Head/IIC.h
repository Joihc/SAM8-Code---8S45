#ifndef _IIC_H_
#define _IIC_H_

#include "config.h"
#include "time.h"

#ifndef _IIC_C_
#endif

#define tt_5us (delayus(1))//—” ±5us ±£÷§∂¡–¥

#define SDA_1 (Set_Bit(P2,1))
#define SDA_0 (Clr_Bit(P2,1))
#define SDA_T (IIC_SDA())

#define SCL_1 (Set_Bit(P2,0))
#define SCL_0 (Clr_Bit(P2,0))

short IIC_SDA();
void IIC_Start();
void IIC_Stop();
void IIC_Ack();
void IIC_NoAck();
short IIC_GetACK();
uint8 IIC_SendByte(uint8 Data);
uint8 IIC_RecByte();

#endif /* _IIC_H_ */
