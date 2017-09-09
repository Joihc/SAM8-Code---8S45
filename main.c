//*****************************
//  author：  hg
//  date:     2016/1/19
//  mcu:      s3f9488
//  project:  电磁炉  8MHZ
//*****************************
#define _MAIN_C_


#include "main.h"
#include "adc.h"
#include "PWM.h"

#ifdef Screen_74HC164
#include "74HC164.h"
#elif defined  Screen_TM1629
#include "TM1629.h"
#endif
//003EH -> 1 01011 11   003FH ->1111 1 110   D3
__root const volatile uint8 Smart_Code[]@0x003C={0xFF, 0xFF, 0x84, 0x9F};
__code const uint8 POWER_RATE[] =
{
  0,
  RANGE_NUM*0.2f,
  RANGE_NUM*0.4f,
  RANGE_NUM*0.5f,
  RANGE_NUM*0.6f,
  RANGE_NUM*0.7f,
  RANGE_NUM*0.8f,
  RANGE_NUM*0.9f,
  RANGE_NUM*1,
};
uint4 firstOpen = TRUE;
uint4 haveViewSet = FALSE;
uint8 count_60ms = 0;
uint8 count_1s = 0;
uint8 count_2s = 0;

uint8 buzzTime = 0;		//蜂鸣器鸣叫时间

uint8 fanTime = FAN_ALL_TIME;		//风扇旋转时间 0表示不计时状态 >0表示计时状态

uint4 rangeNow = 0;			//当前档位

uint16 statusViewNum = 0;		//每位检测到状态表示左到右,1表示故障0表示正常  0无锅/1线盘超温/2线盘开路/3IGBT1超温/4IGBT1开路/5IGBT2超温/6IGBT2开路
								//7电压低/8电压高/9缺相/10档位开关开路/11锅底开路/12锅底超温/13IGBT驱动故障/14输出互感器反/15线盘不通
uint4 nullPot = 0;//检测无锅次数
uint4 vLow = 0;   //电压低次数
uint4 vLowOut =0;
uint4 vHight = 0; //电压高次数
uint4 vHightOut =0;
//uint4 vCut = 0;   //电压缺相次数
uint4 cTransformerCut = 0;//线盘状态

uint4 nulligbtToLay=0;//igbterror退出延迟次数

uint4 checkTimeOn = FALSE;//无延时检测
volatile uint8 nullPotCheckTime = 60;//检锅延时
volatile uint8 igbtErrorCheckTime = 60;//igbt驱动恢复延时
uint8 temperatureCheckTime = 40;//温度检测延时


uint4 checkTransformerCut =0;//线盘时继续线盘
uint4 checkNullPot =0;//无锅时继续无锅

uint4 errorLay=0;


volatile uint4 turnOnLay=0;
uint4 nullPotLay=0;//无锅显示延迟
uint4 nulligbtLay=0;//igbtError显示延迟

//uint4 whiletimes =0;

#ifdef Screen_TM1629
int16 tempurature =0;
#endif

#pragma inline=forced
								   //--initiation
void sysInit()
{
	BTCON = 0xFB;
	CLKCON = 0x18;
        //FLAGS
        //RP0
        //RP1
        //SPH
        //SPL
        //IPH
        //IPL
        //IRQ
        IMR = 0x41;//只开启IRQ6(P3.3-P3.6)和IRQ(TA M/C)
	//SYM = 0x01;
        
        RESETID = 0x17;
        //TD0CON
        //TD1CON
        ADCON = 0xF6;
        
        TADATA = 234;
	TACON = 0x06;
        
	//OSCCON = 0x04;

        TBCON = 0x73;//0x01   11 0111  /2  关闭STOP TIMER B
        OSCCON = 0x04;
        
	INTPND = 0x00;
        IPR =0xA0;
        WDTCON = 0xAA;
}

#pragma inline=forced
void ioInit()
{
	/* PO I/O口 不用IO口均设置为输入上拉 默认*/
        P0CONH = 0x8A;//10(ADC4) 00(温控) 10(ADC6) 10(ADC7)
	
	P0CONL = 0x85;//10(ADC8) 00(P0.2/reset) 01(P0.1/XTOUT) 01(P0.0/FAN)
	P0PUR = 0x07;//0 0 0 0 0   0 1 1 1
				 /* P1 I/O口 */
	P1CONH = 0x05;//00(P1.7) 00(P1.6) 01(P1.5/蜂鸣器) 01(P1.4/A316J低电平复位信号)
	P1CONL = 0xAB;//10(P1.3/ADC0) 10(P1.2/ADC1) 10(P1.1/ADC2) 11(P1.0/PWM)
	P1PUR = 0xF1;//1 1 1 1  0 0 0 1
				 /*  P2 I/0口*/
	P2CONH = 0x15;//00(P2.7) 01(P2.6/LED) 01(P2.5/164 DAT) 01(P2.4/164 CLK)
#ifdef Screen_74HC164
	P2CONL = 0x55;//01(P2.3/COM1) 01(P2.2/COM2) 01出 00入(P2.1/IIC-SDA)01(P2.0/IIC-SCL)
#elif defined Screen_TM1629
	P2CONL = 0x55;//01(P2.3/COM1) 01(P2.2/COM2) 01(P2.1/IIC-SDA) 01(P2.0/IIC-SCL)
#endif

				  /*  P3 I/0口*/
	P3CONH = 0x14;//00     01(P3.6/SCLK) 01(P3.5/SDAT) 00(INT1 P3.4/下降) 
	P3CONL = 0x15;//00(INT0 P3.3/下降沿) 01(P3.2) 01(P3.1) 01(P3.0)
        P3PUR  = 0xC0;//1100 0000

        P03INTH = 0x00;//0000
        P03INTL = 0x05;//0000 0101
	P03PND = 0x00;//0000 1(P3.6/SCLK) 0(P3.5/SDAT) 0(P3.4/下降沿) 0(P3.3/下降沿) -中断挂起 置0重置
				 /*  P4 I/0口 全部挂起*/
}
#pragma inline=forced
void defaultValue()
{
	firstOpen = TRUE;
	haveViewSet = FALSE;
	count_60ms = 0;
	count_1s = 0;
	count_2s = 0;
        
	buzzTime = 0;		//蜂鸣器鸣叫时间

	fanTime = FAN_ALL_TIME;		//风扇旋转时间 0表示不计时状态 >0表示计时状态

	rangeNow = 0;			//当前档位

	statusViewNum = 0;		//每位检测到状态表示左到右,1表示故障0表示正常  0无锅/1线盘超温/2线盘开路/3IGBT1超温/4IGBT1开路/5IGBT2超温/6IGBT2开路
							//7电压低/8电压高/9缺相/10档位开关开路/11锅底开路/12锅底超温/13IGBT驱动故障/14输出互感器反/15线盘不通
	nullPot = 0;//检测无锅次数
	vLow = 0;   //电压低次数
	vHight = 0; //电压高次数
	//vCut = 0;   //电压缺相次数
	cTransformerCut = 0;//线盘状态

        nulligbtToLay=0;//igbterror退出延迟次数

	nullPotCheckTime = 60;//检锅延时
	temperatureCheckTime = 40;//温度检测延时40s
        
        turnOnLay =0;
        nullPotLay =0;
        nulligbtLay =0;
        
}
int main()
{
	di;
	//系统初始化 IO初始化
	sysInit();
	ioInit();

#ifdef Screen_74HC164
	init_74HC164();
#elif defined Screen_TM1629
	init_TM1629();
#endif

	defaultValue();
        init_adc();
        initPWM();
	ei;
        BUZZ_ON;
        CLEAR_WD;
        while(turnOnLay<TURN_ALL_TIME)
        {
          CLEAR_WD;
        };
        rangeNow = getSwitchByAnum();   
#ifdef Screen_TM1629
        tempurature = getTemperatureByAnum(7);
#endif


	while (1)
	{
#ifdef DEBUG
		CLEAR_WD;
		//SwitchSet();
		//ViewSet(rangeNow);
                //P1CONL = 0xFC;
                //Set_Bit(P1,0);
                //AJ_ON;
                SwitchSet();
                fixPWM(rangeNow);
                ViewSet(rangeNow);
#else
                CLEAR_WD;
                
                
		haveViewSet = FALSE;
		checkTimeOn = FALSE;
		SwitchSet();//设置档位
		
                  updata_vol();
		  DetectCoilHot();//线盘超温
		  DetectCoilCut();//线盘探头开路
		  DetectIGBTHot_1();//IGBT超温
		  DetectIGBTCut_1();//IGBT探头开路
		  DetectIGBTHot_2();//IGBT超温
                  CLEAR_WD;
		  DetectIGBTCut_2();//IGBT探头开路
		  DetectVLow();//低压检测
		  DetectVHight();//高压检测
		  //DetectVCut();//缺相检测
		  DetectSwitchCut();//档位开关开路
		  DetectUnderPotCut();//锅底探头开路
		  DetectUnderPotHot();//锅底超温*/
                if(PWM_OPEN)//只在开通状态下检查TBCON == 0x77
                {                                         
                  DetectTransformerCut();//线盘断了或者输出互感器坏了
		  DetectIgbtError();//IGBT驱动故障
                  DetectNullPot();//无锅检测 
                }

                CLEAR_WD;
		  //低压
		  if ((statusViewNum & ((uint16)1 << 7)) && !haveViewSet)
		  {
			ViewSet(108);
			haveViewSet = TRUE;
		  }
		  //高压
		  if ((statusViewNum & ((uint16)1 << 8)) && !haveViewSet)
		  {
			ViewSet(109);
			haveViewSet = TRUE;
		  }
		  //缺相
		  if ((statusViewNum & ((uint16)1 << 9)) && !haveViewSet)
		  {
			ViewSet(108);
			haveViewSet = TRUE;
		  }
		  //档位开路
		  if ((statusViewNum & ((uint16)1 << 10)) && !haveViewSet)
		  {
			ViewSet(113);
			haveViewSet = TRUE;
		  }
		  //线盘开路
		  if ((statusViewNum & ((uint16)1 << 2)) && !haveViewSet && !temperatureCheckTime)
		  {
			ViewSet(103);
			haveViewSet = TRUE;
		  }
		  //IGBT1探头开路
		  if ((statusViewNum & ((uint16)1 << 4)) && !haveViewSet && !temperatureCheckTime)
		  {
			ViewSet(105);
			haveViewSet = TRUE;
		  }
		  //IGBT2探头开路
		  if ((statusViewNum & ((uint16)1 << 6)) && !haveViewSet && !temperatureCheckTime)
		  {
			ViewSet(105);
			haveViewSet = TRUE;
		  }
		//锅底探头开路
		if ((statusViewNum & ((uint16)1 << 11)) && !haveViewSet && !temperatureCheckTime)
		{
			ViewSet(107);
			haveViewSet = TRUE;
		}
                CLEAR_WD;
		if (rangeNow == 0)
		{
			//为0档位时
			firstOpen = FALSE;
			if (fanTime == 0)
			{
				fanTime = 1;//等待关闭风机
			}
			if (!haveViewSet)
			{
				ViewSet(0);
			}
			fixPWM(0);
                        nullPot =0;//无锅次数
	                cTransformerCut = 0;//线盘状态
                        //重置故障
                        statusViewNum &= ~((uint16)1 << 0);//无锅 正常
                        statusViewNum &= ~((uint16)1 << 1);//线盘超温置0 正常
                        statusViewNum &= ~((uint16)1 << 3);//IGBT1超温置0 正常
                        statusViewNum &= ~((uint16)1 << 5);//IGBT2超温置0 正常
                        statusViewNum &= ~((uint16)1 << 12);//锅底超温置0 正常
                        statusViewNum &= ~((uint16)1 << 13);//IGBT驱动故障置0 正常
                        statusViewNum &= ~((uint16)1 << 15);//线盘不通或者输出互感器损坏置0 正常
		}
		else
		{
			//线盘超温
			if ((statusViewNum & ((uint16)1 << 1)) && !haveViewSet)
			{
				ViewSet(102);
				haveViewSet = TRUE;
			}
			//IGBT1超温
			if ((statusViewNum & ((uint16)1 << 3)) && !haveViewSet)
			{
				ViewSet(104);
				haveViewSet = TRUE;
			}
			//IGBT2超温
			if ((statusViewNum & ((uint16)1 << 5)) && !haveViewSet)
			{
				ViewSet(104);
				haveViewSet = TRUE;
			}
			//锅底超温
			if ((statusViewNum & ((uint16)1 << 12)) && !haveViewSet)
			{
				ViewSet(106);
				haveViewSet = TRUE;
			}
                        //线盘不通或者输出互感器损坏
                        if((statusViewNum & ((uint16)1 << 15)) && !haveViewSet)
                        {                        
                            ViewSet(110);
                            haveViewSet = TRUE;
                            nullPot =0;//无锅次数
                        }
			//IGBT驱动故障
			if ((statusViewNum & ((uint16)1 << 13)) && !haveViewSet && !checkTimeOn)
			{
                                if(nulligbtLay<3)
                                {
                                  ViewSet(rangeNow);
                                }
                                else
                                {
				  ViewSet(112);
                                }
				haveViewSet = TRUE;
				checkTimeOn = TRUE;
                                if (igbtErrorCheckTime >=DELAY_TIME)
				{
				    igbtErrorCheckTime =0;
                                    if(nulligbtLay <3)
                                    {
                                      nulligbtLay++;
                                    }
				}
				if(FALUT_OPEN && Test_Bit(P3, 3))
				{
                                  if(igbtErrorCheckTime == 0)//时间重置且在复位后的状态
                                  {
                                    fixPWM(rangeNow);
                                    nulligbtToLay =1;
                                    igbtErrorCheckTime++;
                                  }
                                }
                                else
                                {
                                    fixPWM(0);
                                    nulligbtToLay =0;
                                }
                                
                                nullPot =0;//无锅次数
	                        cTransformerCut = 0;//线盘状态
			}
                        //无锅
			if ((statusViewNum & ((uint16)1 << 0)) && !haveViewSet && !checkTimeOn)
			{
                            if(nullPotLay <1)
                            {
                              ViewSet(rangeNow);
                            }
                            else
                            {
                              ViewSet(101);
                            }
				
				haveViewSet = TRUE;
				checkTimeOn = TRUE;
				if (nullPotCheckTime >=DELAY_TIME)
				{
				    nullPotCheckTime = 0;      
                                    checkNullPot =0;
                                    if(nullPotLay<3)
                                    {
                                      nullPotLay++;
                                    }
				}
                                if(checkNullPot<10)
                                {
                                    fixPWM(rangeNow);
                                    checkNullPot++;
                                }
                                else
                                {
                                    fixPWM(0); 
                                }
	                        cTransformerCut = 0;//线盘状态
			}
			if (!haveViewSet)
			{
				if (firstOpen)
				{
					fixPWM(0);//关闭输出
					ViewSet(0);//显示0档位
					if (fanTime == 0)
					{
						fanTime = 1;//等待关闭风机
					}
                                        //0档检测的置0
                                        nullPot =0;//无锅次数
	                                cTransformerCut = 0;//线盘状态
				}
				else
				{
                                        if(Test_Bit(P0,6))
                                        {
                                          fixPWM(0);
                                          nullPot =0;//无锅次数
	                                  cTransformerCut = 0;//线盘状态
                                        }
                                        else
                                        {
                                          fixPWM(rangeNow);//开启输出
                                        }
					ViewSet(rangeNow);//显示档位
					fanTime = 0;//开启风机   
				}
			}
			else
			{
                          	if (firstOpen || !checkTimeOn)
				{
				    fixPWM(0);//关闭输出
                                    nullPot =0;//无锅次数
	                            cTransformerCut = 0;//线盘状态
                                }
				if (fanTime == 0)
				{
					fanTime = 1;//等待关闭风机
				}
			}
		}
#endif
	}
}
#pragma inline=forced
void DetectNullPot()
{
	uint16 temp_2 = (uint16)1 << 0;
        uint4 temp = get_13ADC();// 1表示无锅
	if ((temp != 1) && !(statusViewNum & temp_2))
	{
		//检测到有锅且显示有锅
		nullPot = 0;
		return;
	}
	if ((temp != 1) && (statusViewNum & temp_2))
	{
		//检测到有锅且显示无锅
                delay(2);
                if(get_13ADC()==1)
                  return;
		statusViewNum &= ~temp_2;//置0 正常
		nullPot = 0;
		return;
	}
	if ((temp == 1) && (statusViewNum & temp_2))
	{
		//检测到无锅且无锅
                nullPot=0;
		return;
	}
	if ((temp == 1) && !(statusViewNum & temp_2))
	{
		//检测到无锅且有锅
                nullPot++;
		if (nullPot >=10)
		{
			nullPot = 0;
                        nullPotCheckTime =0;
                        nullPotLay =0;
                        checkNullPot =0;
			statusViewNum |= temp_2;//置1 无锅状态
		}
	}
}
#pragma inline=forced
void DetectCoilHot()
{
	uint16 temp_2 = (uint16)1 << 1;
	uint4 temp = get_07ADC();//1 温高
	if ((temp != 1) && !(statusViewNum & temp_2))
	{
		//检查温度正常且显示温度正常
		return;
	}
	if ((temp != 1) && (statusViewNum & temp_2))
	{
		//检查温度正常且显示温度不正常
		return;
	}
	if ((temp == 1) && (statusViewNum & temp_2))
	{
		//检查温度不正常且显示温度不正常
		return;
	}
	if ((temp == 1) && !(statusViewNum & temp_2))
	{
		//检查温度不正常且显示温度正常
		delay(2);//延时2ms
		if (get_07ADC() != 1)
			return;
	        statusViewNum |= temp_2;//置1 不正常
	}

}
#pragma inline=forced
void DetectCoilCut()
{
	uint16 temp_2 = (uint16)1 << 2;
	uint4 temp = get_07ADC();//2 探头开路
	if ((temp != 2) && !(statusViewNum & temp_2))
	{
		//检查正常且显示正常
		return;
	}
	if ((temp != 2) && (statusViewNum & temp_2)&& errorLay>=4)
	{
		//检查正常且显示不正常
		delay(2);//延时2ms
		if (get_07ADC() == 2)
			return;
		statusViewNum &= ~temp_2;//置0 正常
		return;
	}
	if ((temp == 2) && (statusViewNum & temp_2))
	{
		//检查不正常且显示不正常
		return;
	}
	if ((temp == 2) && !(statusViewNum & temp_2))
	{
		//检查不正常且显示正常
		delay(2);//延时2s
		if (get_07ADC() != 2)
			return;
			statusViewNum |= temp_2;//置1 不正常
                errorLay = 0;
	}
}
#pragma inline=forced
void DetectIGBTHot_1()
{
	uint16 temp_2 = (uint16)1 << 3;
	uint4 temp = get_04ADC();//2高温
	if ((temp != 2) && !(statusViewNum & temp_2))
	{
		//正常且正常
		return;
	}
	if ((temp != 2) && (statusViewNum & temp_2))
	{
		//正常且不正常
		return;
	}
	if ((temp == 2) && (statusViewNum & temp_2))
	{
		//不正常且不正常
		return;
	}
	if ((temp == 2) && !(statusViewNum & temp_2))
	{
		//不正常且正常
		delay(2);
		if (get_04ADC() != 2)
			return;
			statusViewNum |= temp_2;//置1 不正常
	}
}
#pragma inline=forced
void DetectIGBTCut_1()
{
	uint16 temp_2 = (uint16)1 << 4;
	uint4 temp = get_04ADC();//1开路
	if ((temp != 1) && !(statusViewNum & temp_2))
	{
		//正常且正常
		return;
	}
	if ((temp != 1) && (statusViewNum & temp_2)&& errorLay>=4)
	{
		//正常且不正常
		delay(2);
		if (get_04ADC() == 1)
			return;
		statusViewNum &= ~temp_2;//置0 正常
		return;
	}
	if ((temp == 1) && (statusViewNum & temp_2))
	{
		//不正常且不正常
		return;
	}
	if ((temp == 1) && !(statusViewNum & temp_2))
	{
		//不正常且正常
		delay(2);
		if (get_04ADC() != 1)
			return;
		statusViewNum |= temp_2;//置1 不正常
                errorLay = 0;
	}
}
#pragma inline=forced
void DetectIGBTHot_2()
{
	uint16 temp_2 = (uint16)1 << 5;
	uint4 temp = get_11ADC();//2高温
	if ((temp != 2) && !(statusViewNum & temp_2))
	{
		//正常且正常
		return;
	}
	if ((temp != 2) && (statusViewNum & temp_2))
	{
		//正常且不正常
		return;
	}
	if ((temp == 2) && (statusViewNum & temp_2))
	{
		//不正常且不正常
		return;
	}
	if ((temp == 2) && !(statusViewNum & temp_2))
	{
		//不正常且正常
		delay(2);
		if (get_11ADC() != 2)
			return;
		statusViewNum |= temp_2;//置1 不正常
	}
}
#pragma inline=forced
void DetectIGBTCut_2()
{
	uint16 temp_2 = (uint16)1 << 6;
	uint4 temp = get_11ADC();//1开路
	if ((temp != 1) && !(statusViewNum & temp_2))
	{
		//正常且正常
		return;
	}
	if ((temp != 1) && (statusViewNum & temp_2)&& errorLay>=4)
	{
		//正常且不正常
		delay(2);
		if (get_11ADC() == 1)
			return;
		statusViewNum &= ~temp_2;//置0 正常
		return;
	}
	if ((temp == 1) && (statusViewNum & temp_2))
	{
		//不正常且不正常
		return;
	}
	if ((temp == 1) && !(statusViewNum & temp_2))
	{
		//不正常且正常
		delay(2);
		if (get_11ADC() != 1)
			return;
		statusViewNum |= temp_2;//置1 不正常
                errorLay = 0;
	}
}
#pragma inline=forced
void DetectVLow()
{
	uint16 temp_2 = (uint16)1 << 7;
        uint4 temp = 0;
	if (statusViewNum & temp_2)
	{
		//电压低
		temp = 1;
	}
	else if (statusViewNum & ((uint16)1 << 8))
	{
		//电压高
		temp = 2;
	}
	temp = get_03ADC(temp);//2 压低
	if ((temp != 1) && !(statusViewNum & temp_2))
	{
		//正常且正常
		vLow = 0;
                vLowOut =0;
		return;
	}
	if ((temp != 1) && (statusViewNum & temp_2)&& errorLay>=4)
	{
		//正常且不正常
                vLowOut++;
                if(vLowOut>4)
                {
		  statusViewNum &= ~temp_2;//置0 正常
                }
		vLow = 0;
		return;
	}
	if ((temp ==1) && (statusViewNum & temp_2))
	{
		//不正常且不正常
		vLow = 0;
                vLowOut =0;
		return;
	}
	if ((temp == 1) && !(statusViewNum & temp_2))
	{
		//不正常且正常
		vLow++;
		if (vLow >= 3)
		{
			vLow = 0;
			statusViewNum |= temp_2;//置1 不正常
		}
                vLowOut =0;
                errorLay = 0;
	}
}
#pragma inline=forced
void DetectVHight()
{
	uint16 temp_2 = (uint16)1 << 8;
        uint4 temp =0;
	if ((statusViewNum & ((uint16)1 << 7)))
	{
		//电压低
		temp = 1;
	}
	else if ((statusViewNum & temp_2))
	{
		//电压高
		temp = 2;
	}
	temp = get_03ADC(temp);//2 压低
	if ((temp != 2) && !(statusViewNum & temp_2))
	{
		//正常且正常
		vHight = 0;
                vHightOut =0;
		return;
	}
	if ((temp != 2) && (statusViewNum & temp_2)&& errorLay>=4)
	{
		//正常且不正常
                vHightOut++;
                if(vHightOut>4)
                {
		  statusViewNum &= ~temp_2;//置0 正常
                }
		vHight = 0;
		return;
	}
	if ((temp == 2) && (statusViewNum & temp_2))
	{
		//不正常且不正常
		vHight = 0;
                vHightOut =0;
		return;
	}
	if ((temp == 2) && !(statusViewNum & temp_2))
	{
		//不正常且正常
		vHight++;
		if (vHight >= 3)
		{
			vHight = 0;
			statusViewNum |= temp_2;//置1 不正常
		}
                vHightOut =0;
                errorLay = 0;
	}
}
#pragma inline=forced
void DetectSwitchCut()
{
	uint16 temp_2 = (uint16)1 << 10;
	uint4 temp = get_05ADC();//1 开路
	if ((temp != 1) && !(statusViewNum & temp_2))
	{
		//正常且正常
		return;
	}
	if ((temp != 1) && (statusViewNum & temp_2)&& errorLay>=4)
	{
		//正常且不正常
		delay(2);
		if (get_05ADC() == 1)
			return;
		statusViewNum &= ~temp_2;//置0 正常
		return;
	}
	if ((temp == 1 )&& (statusViewNum & temp_2))
	{
		//不正常且不正常
		return;
	}
	if ((temp == 1) && !(statusViewNum & temp_2))
	{
		//不正常且正常
		delay(2);
		if (get_05ADC() != 1)
			return;
		statusViewNum |= temp_2;//置1 不正常
                errorLay = 0;
	}
}
#pragma inline=forced
void DetectUnderPotCut()
{
	uint16 temp_2 = (uint16)1 << 11;
	uint4 temp = get_06ADC();//2 锅底探头开路
	if ((temp != 2) && !(statusViewNum & temp_2))
	{
		//正常且正常
		return;
	}
	if ((temp != 2) && (statusViewNum & temp_2) && errorLay>=4)
	{
		//正常且不正常
		delay(2);
		if (get_06ADC() == 2)
			return;
		statusViewNum &= ~temp_2;//置0 正常
		return;
	}
	if ((temp == 2) && (statusViewNum & temp_2))
	{
		return;
	}
	if ((temp == 2) && !(statusViewNum & temp_2))
	{
		//不正常且正常
		delay(2);
		if (get_06ADC() != 2)
			return;
		statusViewNum |= temp_2;//置1 不正常
                errorLay = 0;
	}
}
#pragma inline=forced
void DetectUnderPotHot()
{
	uint16 temp_2 = (uint16)1 << 12;
	uint4 temp = get_06ADC();//1超温
	if ((temp != 1) && !(statusViewNum & temp_2))
	{
		//正常且正常
		return;
	}
	if ((temp != 1) && (statusViewNum & temp_2))
	{
		//正常且不正常
		return;
	}
	if ((temp == 1) && (statusViewNum & temp_2))
	{
		return;
	}
	if ((temp == 1 )&& !(statusViewNum & temp_2))
	{
		//不正常且正常
		delay(2);
		if (get_06ADC() != 1)
		return;
		statusViewNum |= temp_2;//置1 不正常
	}
}
#pragma inline=forced
void DetectIgbtError()
{
	uint16 temp_2 = (uint16)1 << 13;
	uint4 temp = Test_Bit(P3, 3);//0不正常
	if (FALUT_OPEN && temp && !(statusViewNum & temp_2))
	{
		//正常且正常
		return;
	}
	if (FALUT_OPEN && temp && (statusViewNum & temp_2))
	{
		//正常且不正常
                if(nulligbtToLay>=4)
                {
                    statusViewNum &= ~temp_2;//置0 正常
                }
                else if(nulligbtToLay>=1)
                {
                  nulligbtToLay++;
                }
		return;
	}
	if ((!FALUT_OPEN || !temp)&& (statusViewNum & temp_2))
	{
		//不正常且不正常
		return;
	}
	if ((!FALUT_OPEN || !temp)&& !(statusViewNum & temp_2))
	{
		//不正常且正常
                statusViewNum |= temp_2;//置1 不正常
                nulligbtLay =0;
                nulligbtToLay =0;//重置故障恢复时间
	}

}
#pragma inline=forced
void DetectTransformerCut()
{
	uint16 temp_2 = (uint16)1 << 15;
	uint16 temp = getADCNum(12);//0表示线盘断了
	if ((temp>=5)&& !PWMChange()&& !(statusViewNum & temp_2))
	{
		//正常且正常
		cTransformerCut = 0;
		return;
	}
	if ((temp>=5)&& !PWMChange()&& (statusViewNum & temp_2))
	{
		//正常且不正常
		//delay(2);
		//if (getADCNum(12)<5)
		//	return;
		//statusViewNum &= ~temp_2;//置0 正常
		cTransformerCut = 0;
		return;
	}
	if (((temp<5)|| PWMChange()) && (statusViewNum & temp_2))
	{
		//不正常且不正常
		//delay(2);
                cTransformerCut = 0;
		return;
	}
	if (((temp<5)|| PWMChange())&& !(statusViewNum & temp_2))
	{
		//不正常且正常
		cTransformerCut++;
		if (cTransformerCut >= 30)
		{
			cTransformerCut = 0;
			statusViewNum |= temp_2;//置1 不正常
		}
                nullPot = 0;
	}

}
#pragma inline=forced
void SwitchSet()
{
	uint4 rangeNext = getSwitchs();
	if (rangeNext != rangeNow && rangeNext != 9)
	{
          rangeNow=rangeNext;
          BUZZ_ON;
	}
}
#ifdef Screen_74HC164
void ViewSet(uint8 ShowNum)
{
	setNum_74HC164(ShowNum);
	whileUpdate_74HC164();
}
#elif defined Screen_TM1629
void ViewSet(uint8 ShowNum)
{
        int16 tempnum =0;
        if(ShowNum>100)
        {
          set_TM1629_Up(ShowNum);
        }
        else
        {
           set_TM1629_Up(POWER_RATE[ShowNum]);
        }
	set_TM1629_LeftNum(rangeNow);
        set_TM1629_Leftstring(getPWMRate());
	if (ShowNum<100 && ShowNum>0)//温度模式
	{
                tempnum =getTemperatureByAnum(7);
          	if(tempurature+3<tempnum)
                {
                  tempurature++;
                }
                else if(tempurature-3>tempnum)
                {
                  tempurature--;
                }
		// tempurature =getTemperatureByAnum(6);//锅底温度
		//getADCNum(13) 输入互感器电流 AD
		//getADCNum(12) 输出互感器电流大小
		set_TM1629_Down(tempurature, 1);            
	}
	else//时间模式
	{
		set_TM1629_Down(0, 0);
                //set_TM1629_Down(getVo(), 1);
	}
	whileUpdate_TM1629();
}
#endif
//中断  __TA_OVERFLOW_vector
#pragma vector=__TA_MATCH_CAPTURE_vector
__interrupt void TA_MATCH_CAPTURE_vector()
{
  //if(INTPND & 0x02 != 0 )
  //{
    TAInterupt();
  //  INTPND &= ~0x02;
  //}
}
#pragma vector=__P33_EXTERNAL_INT_vector
__interrupt void P33INT_vector()
{
  if(P03PND & 0x01)
  {
    //关闭输出
    if(PWM_OPEN)//在开启状态
    {
      P03INTL &= 0xFC;//关闭中断
      closePWM();//关闭输出
    }
    P03PND &= ~0x01;
  }
}
#pragma vector=__P34_EXTERNAL_INT_vector
__interrupt void P34INT_vector()
{
  if(P03PND & 0x02)
  {
    //相位补偿
    if(PWM_OPEN)//在开启状态
    {
      P03INTL &= 0xF3;//关闭中断 
    }
    P03PND &= ~0x02;
  }
}
#pragma inline=forced
//A定时器
void TAInterupt()
{
        CLEAR_WD;
	count_60ms++;
        if(nullPotCheckTime <DELAY_TIME)
        {
          nullPotCheckTime++;
        }
        if(igbtErrorCheckTime < DELAY_TIME)
        {
          igbtErrorCheckTime++;
        }     
	if (BUZZ_Test)
	{
		buzzTime++;
		if (buzzTime >= BUZZ_ALL_TIME)
		{
			buzzTime = 0;
			BUZZ_OFF;
		}
		else
		{
			BUZZ_ON;
		}
	}
	if (count_60ms == 2)//60ms
	{
		count_60ms = 0;
		count_2s++;
		count_1s++;

		if (count_2s == 34)//2s
		{
			count_2s = 0;

			if (fanTime >= 1 && fanTime<FAN_ALL_TIME)
			{
				fanTime++;
			}

			if (fanTime <FAN_ALL_TIME - 5)
			{
				FAN_ON;
			}
			else
			{
				FAN_OFF;
			}
		}
		if (count_1s == 17)
		{
			count_1s = 0;
                        if(errorLay<6)
                        {
                          errorLay++;
                        }
                        if(turnOnLay<TURN_ALL_TIME)
                        {
                          turnOnLay++;
                        }
			Com_Bit(P2, 6);
			//INTERUPT 更新区域
#ifdef Screen_74HC164
			interuptUpdate_74HC164();
#elif defined Screen_TM1629
			interuptUpdate_TM1629();
#endif
                    if(PWM_OPEN && temperatureCheckTime)//只在开通状态检查温度运转
                    {
                      temperatureCheckTime--;//开路延时倒计时
                    }
		}
	}
}
