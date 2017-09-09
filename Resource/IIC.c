#define _IIC_C_

#include "IIC.h"
/*
  使用AT24C01  128*1字节内存
*/

short IIC_SDA()
{
  short sda =0;
  #ifdef Screen_74HC164
    P2CONL = 0xA2;
  #endif
  #ifdef Screen_TM1629
    P2CONL = 0x82;
  #endif
  sda = Test_Bit(P2,1);
  #ifdef Screen_74HC164
    P2CONL = 0xAA;
  #endif
  #ifdef Screen_TM1629
    P2CONL = 0x8A;
  #endif
  return sda;
}

//起始位
void IIC_Start()
{
	SDA_1;			//SDA=1
	SCL_1;				//SCL=1
	tt_5us;				//4.7us(TSU.STA)
	SDA_0;				//SDA =0
	tt_5us;				//4us(THD.STA)
	SCL_0;				//SCL =0
}
//停止位
void IIC_Stop()
{
	SDA_0;				//SDA =0;
	SCL_1;				//SCL =1;
	tt_5us;				//4.7us(TSU.STA)
	SDA_1;				//SDA =1;
	tt_5us;				//4.7us(TBUF)
}
//主机应答
void IIC_Ack()
{
	SDA_0;				//SDA =0;
	tt_5us;				//0.2us(TSU.DAT)
	SCL_1;				//SCL =1;
	tt_5us;				//4.0us(THIGH)
	SCL_0;				//SCL =0;
	tt_5us;				//4.7us(TLOW)可略
}
//主机反向应答位
void IIC_NoAck()
{
	SDA_1;				//SDA =1;
	tt_5us;
	SCL_1;				//SCL =1;
	tt_5us;
	SCL_0;				//SCL =0;
	tt_5us;
}
//获取从机应答信号，返回0时收到ACK，返回1时没收到
short IIC_GetACK()
{
	short ErrorBit;
	SDA_1;				//SDA =1;
	tt_5us;				//0.2us(TSU.DAT)
	SCL_1;				//SCL =1;
	tt_5us;				//4.0us(THIGH)
	ErrorBit = SDA_T;			//ErrorBit = SDA;
	SCL_0;                                //SCL =0;
	tt_5us;				//4.7us(TLOW) 可略
	return ErrorBit;
}
/*
	IC向从设备发送一字节
	0 - successful 1-fault
*/
uint8 IIC_SendByte(uint8 Data)
{
	uint8 i;				//位控制
	for(i=0;i<8;i++)			//写入时是时钟下降沿同步数据
	{
		if(Data & 0x80)
			SDA_1;		//SDA =1;
		else
			SDA_0;		//SDA =0;
		tt_5us;			//0.2us(TSU.DAT)
		SCL_1;			//SCL =1;
		tt_5us;			//4.0us(THIGH)
		SCL_0;			//SCL =0;
		tt_5us;			//4.7us(TLOW)
		Data<<=1;
	}
	return IIC_GetACK();
}
/*
	IC向从设备读取一字节
	返回读取字节
*/
uint8 IIC_RecByte()
{
	uint8 i,rbyte=0;
	SDA_1;				//SDA =1;
	for(i=0;i<8;i++)
	{
		SCL_0;			//SCL =0;
		tt_5us;			//4.7us(TLOW)
		SCL_1;			//SCL =1;
		tt_5us;			//4.0us(THIGH)
		if(SDA_T)			//if(SDA)
			rbyte|=(0x80>>i);
	}
	SCL_0;				//SCL =0;
	return rbyte;

}
