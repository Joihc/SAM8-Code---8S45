#define _IIC_C_

#include "IIC.h"
/*
  ʹ��AT24C01  128*1�ֽ��ڴ�
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

//��ʼλ
void IIC_Start()
{
	SDA_1;			//SDA=1
	SCL_1;				//SCL=1
	tt_5us;				//4.7us(TSU.STA)
	SDA_0;				//SDA =0
	tt_5us;				//4us(THD.STA)
	SCL_0;				//SCL =0
}
//ֹͣλ
void IIC_Stop()
{
	SDA_0;				//SDA =0;
	SCL_1;				//SCL =1;
	tt_5us;				//4.7us(TSU.STA)
	SDA_1;				//SDA =1;
	tt_5us;				//4.7us(TBUF)
}
//����Ӧ��
void IIC_Ack()
{
	SDA_0;				//SDA =0;
	tt_5us;				//0.2us(TSU.DAT)
	SCL_1;				//SCL =1;
	tt_5us;				//4.0us(THIGH)
	SCL_0;				//SCL =0;
	tt_5us;				//4.7us(TLOW)����
}
//��������Ӧ��λ
void IIC_NoAck()
{
	SDA_1;				//SDA =1;
	tt_5us;
	SCL_1;				//SCL =1;
	tt_5us;
	SCL_0;				//SCL =0;
	tt_5us;
}
//��ȡ�ӻ�Ӧ���źţ�����0ʱ�յ�ACK������1ʱû�յ�
short IIC_GetACK()
{
	short ErrorBit;
	SDA_1;				//SDA =1;
	tt_5us;				//0.2us(TSU.DAT)
	SCL_1;				//SCL =1;
	tt_5us;				//4.0us(THIGH)
	ErrorBit = SDA_T;			//ErrorBit = SDA;
	SCL_0;                                //SCL =0;
	tt_5us;				//4.7us(TLOW) ����
	return ErrorBit;
}
/*
	IC����豸����һ�ֽ�
	0 - successful 1-fault
*/
uint8 IIC_SendByte(uint8 Data)
{
	uint8 i;				//λ����
	for(i=0;i<8;i++)			//д��ʱ��ʱ���½���ͬ������
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
	IC����豸��ȡһ�ֽ�
	���ض�ȡ�ֽ�
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
