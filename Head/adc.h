#ifndef _ADC_H_
#define _ADC_H_

#include "config.h"
#include "time.h"
#include "AT24C01.h"

#ifndef _ADC_C_
/*
  10位ADC输出数字量：ADValue = (模拟量/基准电压)*1023
  12位ADC输出数字量：ADValue = (模拟量/基准电压)*4095
  17位ADC输出数字量：ADValue = (模拟量/基准电压)*131071
*/
//extern uint16 adc10 = 1023;
/*
  无锅检测相关
*/

#endif

#define NULL_NUM (1000)//开路数字  4.98V
#define FILTER_N (10) //采样个数
#define SWITCH_AREA (5)//开关模糊区

//96 172 237 294 384 455 512 578 630

#define S_0 (96)  //96 172 237 294 384 455 512 578 630    123K
#define S_1 (172) // 96 172 235 294 335 385 445 505 545 585  1 2K
#define S_2 (237) 
#define S_3 (294) 
#define S_4 (384) 
#define S_5 (455) 
#define S_6 (512) 
#define S_7 (578) 
#define S_8 (630) 

#define VOL_LOW (443)
#define VOL_HIGH (743)//717
#define VOL_LENGTH (10)
#define AREA (2)//自身模糊区大小

void init_adc();
void updata_vol();
uint16 getVo();

uint4 get_12ADC();
uint4 get_03ADC(uint4 last_index);
uint4 get_05ADC();
uint4 get_04ADC();
uint4 get_11ADC();
uint4 get_07ADC();
uint4 get_06ADC();
uint4 get_13ADC();

//uint16 getADCNumByNum(uint8 IO_P);
uint16 getADCNum(uint8 IO_P);
int16 getTemperatureByAnum(uint8 IO_P);

uint4 getSwitchs();
uint4 getSwitchByAnum();
void writeToAT24C1();

uint8 TestAT24C01();

uint16 getVADCNum();

#endif /* _ADC_H_ */
