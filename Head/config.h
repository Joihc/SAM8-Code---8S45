#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "ioS3F8S45.h"
#include "intrinsics.h"

#define nop (__no_operation())
#define di (__disable_interrupt())
#define ei (__enable_interrupt())

//****************************������д******************************
typedef signed short int4;
typedef signed char int8;//8λ�з�������
typedef signed int int16;//16λ�з�������
typedef signed long int32;//32λ�з�������
typedef unsigned short uint4;
typedef unsigned char uint8;//8λ�޷���
typedef unsigned int uint16;//16λ�޷���
typedef unsigned long unit32;//32λ�޷���
//******************************************************************

//*****************************λ��*********************************
#define Set_Bit(byte,bit) (byte|=(1<<bit))//��byte��bitλ��ȡ1
#define Clr_Bit(byte,bit) (byte&=~(1<<bit))//��byte��bitλ��ȡ0
#define Com_Bit(byte,bit) (byte^=(1<<bit))//��byte��bitλ��ȡ��
#define Test_Bit(byte,bit) (byte&(1<<bit))//��byte��bit���
//******************************************************************


#define Screen_TM1629 // Screen_74HC164 ���� Screen_TM1629

#define RANGE_NUM 50 //P_12KW P_15KW P_20KW P_25KW P_30KW P_35KW P_50KW

//#define DEBUG //debugģʽ�²���飬ֻ�������� DEBUG

#define CLEAR_WD (WDTCON = 0xAA)
#define DELAY_TIME 100
#define BUZZ_ON  (Set_Bit(P1,5))
#define BUZZ_OFF (Clr_Bit(P1,5))
#define BUZZ_Test  (Test_Bit(P1,5))

#define FAN_ON  (Set_Bit(P0,0))
#define FAN_OFF (Clr_Bit(P0,0))

#define AJ_ON  (Set_Bit(P1,4))
#define AJ_OFF  (Clr_Bit(P1,4))
#define AJ_TEST (Test_Bit(P1,4))

#define TRUE 1
#define FALSE 0

#define BUZZ_ALL_TIME 4 //����������ʱ��
#define FAN_ALL_TIME 60 //60����ת��
#define TURN_ALL_TIME 2 //2S����ʱ��

#define FALUT_OPEN (P03INTL & 0x03)//�����жϿ��� 33
#define FIX_OPEN (P03INTL & 0x0C)//��λ�����жϿ��� 34
#define PWM_OPEN (TBCON == 0x77) //�Ƿ�������

#endif /* _CONFIG_H_ */
