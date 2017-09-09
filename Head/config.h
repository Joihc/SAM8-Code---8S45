#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "ioS3F8S45.h"
#include "intrinsics.h"

#define nop (__no_operation())
#define di (__disable_interrupt())
#define ei (__enable_interrupt())

//****************************类型缩写******************************
typedef signed short int4;
typedef signed char int8;//8位有符号整型
typedef signed int int16;//16位有符号整型
typedef signed long int32;//32位有符号整型
typedef unsigned short uint4;
typedef unsigned char uint8;//8位无符号
typedef unsigned int uint16;//16位无符号
typedef unsigned long unit32;//32位无符号
//******************************************************************

//*****************************位域*********************************
#define Set_Bit(byte,bit) (byte|=(1<<bit))//对byte的bit位置取1
#define Clr_Bit(byte,bit) (byte&=~(1<<bit))//对byte的bit位置取0
#define Com_Bit(byte,bit) (byte^=(1<<bit))//对byte的bit位置取反
#define Test_Bit(byte,bit) (byte&(1<<bit))//对byte的bit结果
//******************************************************************


#define Screen_TM1629 // Screen_74HC164 或者 Screen_TM1629

#define RANGE_NUM 50 //P_12KW P_15KW P_20KW P_25KW P_30KW P_35KW P_50KW

//#define DEBUG //debug模式下不检查，只正常运行 DEBUG

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

#define BUZZ_ALL_TIME 4 //蜂鸣器鸣叫时间
#define FAN_ALL_TIME 60 //60秒风机转动
#define TURN_ALL_TIME 2 //2S开机时间

#define FALUT_OPEN (P03INTL & 0x03)//故障中断开启 33
#define FIX_OPEN (P03INTL & 0x0C)//相位补偿中断开启 34
#define PWM_OPEN (TBCON == 0x77) //是否在运行

#endif /* _CONFIG_H_ */
