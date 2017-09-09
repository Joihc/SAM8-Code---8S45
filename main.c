//*****************************
//  author��  hg
//  date:     2016/1/19
//  mcu:      s3f9488
//  project:  ���¯  8MHZ
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

uint8 buzzTime = 0;		//����������ʱ��

uint8 fanTime = FAN_ALL_TIME;		//������תʱ�� 0��ʾ����ʱ״̬ >0��ʾ��ʱ״̬

uint4 rangeNow = 0;			//��ǰ��λ

uint16 statusViewNum = 0;		//ÿλ��⵽״̬��ʾ����,1��ʾ����0��ʾ����  0�޹�/1���̳���/2���̿�·/3IGBT1����/4IGBT1��·/5IGBT2����/6IGBT2��·
								//7��ѹ��/8��ѹ��/9ȱ��/10��λ���ؿ�·/11���׿�·/12���׳���/13IGBT��������/14�����������/15���̲�ͨ
uint4 nullPot = 0;//����޹�����
uint4 vLow = 0;   //��ѹ�ʹ���
uint4 vLowOut =0;
uint4 vHight = 0; //��ѹ�ߴ���
uint4 vHightOut =0;
//uint4 vCut = 0;   //��ѹȱ�����
uint4 cTransformerCut = 0;//����״̬

uint4 nulligbtToLay=0;//igbterror�˳��ӳٴ���

uint4 checkTimeOn = FALSE;//����ʱ���
volatile uint8 nullPotCheckTime = 60;//�����ʱ
volatile uint8 igbtErrorCheckTime = 60;//igbt�����ָ���ʱ
uint8 temperatureCheckTime = 40;//�¶ȼ����ʱ


uint4 checkTransformerCut =0;//����ʱ��������
uint4 checkNullPot =0;//�޹�ʱ�����޹�

uint4 errorLay=0;


volatile uint4 turnOnLay=0;
uint4 nullPotLay=0;//�޹���ʾ�ӳ�
uint4 nulligbtLay=0;//igbtError��ʾ�ӳ�

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
        IMR = 0x41;//ֻ����IRQ6(P3.3-P3.6)��IRQ(TA M/C)
	//SYM = 0x01;
        
        RESETID = 0x17;
        //TD0CON
        //TD1CON
        ADCON = 0xF6;
        
        TADATA = 234;
	TACON = 0x06;
        
	//OSCCON = 0x04;

        TBCON = 0x73;//0x01   11 0111  /2  �ر�STOP TIMER B
        OSCCON = 0x04;
        
	INTPND = 0x00;
        IPR =0xA0;
        WDTCON = 0xAA;
}

#pragma inline=forced
void ioInit()
{
	/* PO I/O�� ����IO�ھ�����Ϊ�������� Ĭ��*/
        P0CONH = 0x8A;//10(ADC4) 00(�¿�) 10(ADC6) 10(ADC7)
	
	P0CONL = 0x85;//10(ADC8) 00(P0.2/reset) 01(P0.1/XTOUT) 01(P0.0/FAN)
	P0PUR = 0x07;//0 0 0 0 0   0 1 1 1
				 /* P1 I/O�� */
	P1CONH = 0x05;//00(P1.7) 00(P1.6) 01(P1.5/������) 01(P1.4/A316J�͵�ƽ��λ�ź�)
	P1CONL = 0xAB;//10(P1.3/ADC0) 10(P1.2/ADC1) 10(P1.1/ADC2) 11(P1.0/PWM)
	P1PUR = 0xF1;//1 1 1 1  0 0 0 1
				 /*  P2 I/0��*/
	P2CONH = 0x15;//00(P2.7) 01(P2.6/LED) 01(P2.5/164 DAT) 01(P2.4/164 CLK)
#ifdef Screen_74HC164
	P2CONL = 0x55;//01(P2.3/COM1) 01(P2.2/COM2) 01�� 00��(P2.1/IIC-SDA)01(P2.0/IIC-SCL)
#elif defined Screen_TM1629
	P2CONL = 0x55;//01(P2.3/COM1) 01(P2.2/COM2) 01(P2.1/IIC-SDA) 01(P2.0/IIC-SCL)
#endif

				  /*  P3 I/0��*/
	P3CONH = 0x14;//00     01(P3.6/SCLK) 01(P3.5/SDAT) 00(INT1 P3.4/�½�) 
	P3CONL = 0x15;//00(INT0 P3.3/�½���) 01(P3.2) 01(P3.1) 01(P3.0)
        P3PUR  = 0xC0;//1100 0000

        P03INTH = 0x00;//0000
        P03INTL = 0x05;//0000 0101
	P03PND = 0x00;//0000 1(P3.6/SCLK) 0(P3.5/SDAT) 0(P3.4/�½���) 0(P3.3/�½���) -�жϹ��� ��0����
				 /*  P4 I/0�� ȫ������*/
}
#pragma inline=forced
void defaultValue()
{
	firstOpen = TRUE;
	haveViewSet = FALSE;
	count_60ms = 0;
	count_1s = 0;
	count_2s = 0;
        
	buzzTime = 0;		//����������ʱ��

	fanTime = FAN_ALL_TIME;		//������תʱ�� 0��ʾ����ʱ״̬ >0��ʾ��ʱ״̬

	rangeNow = 0;			//��ǰ��λ

	statusViewNum = 0;		//ÿλ��⵽״̬��ʾ����,1��ʾ����0��ʾ����  0�޹�/1���̳���/2���̿�·/3IGBT1����/4IGBT1��·/5IGBT2����/6IGBT2��·
							//7��ѹ��/8��ѹ��/9ȱ��/10��λ���ؿ�·/11���׿�·/12���׳���/13IGBT��������/14�����������/15���̲�ͨ
	nullPot = 0;//����޹�����
	vLow = 0;   //��ѹ�ʹ���
	vHight = 0; //��ѹ�ߴ���
	//vCut = 0;   //��ѹȱ�����
	cTransformerCut = 0;//����״̬

        nulligbtToLay=0;//igbterror�˳��ӳٴ���

	nullPotCheckTime = 60;//�����ʱ
	temperatureCheckTime = 40;//�¶ȼ����ʱ40s
        
        turnOnLay =0;
        nullPotLay =0;
        nulligbtLay =0;
        
}
int main()
{
	di;
	//ϵͳ��ʼ�� IO��ʼ��
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
		SwitchSet();//���õ�λ
		
                  updata_vol();
		  DetectCoilHot();//���̳���
		  DetectCoilCut();//����̽ͷ��·
		  DetectIGBTHot_1();//IGBT����
		  DetectIGBTCut_1();//IGBT̽ͷ��·
		  DetectIGBTHot_2();//IGBT����
                  CLEAR_WD;
		  DetectIGBTCut_2();//IGBT̽ͷ��·
		  DetectVLow();//��ѹ���
		  DetectVHight();//��ѹ���
		  //DetectVCut();//ȱ����
		  DetectSwitchCut();//��λ���ؿ�·
		  DetectUnderPotCut();//����̽ͷ��·
		  DetectUnderPotHot();//���׳���*/
                if(PWM_OPEN)//ֻ�ڿ�ͨ״̬�¼��TBCON == 0x77
                {                                         
                  DetectTransformerCut();//���̶��˻����������������
		  DetectIgbtError();//IGBT��������
                  DetectNullPot();//�޹���� 
                }

                CLEAR_WD;
		  //��ѹ
		  if ((statusViewNum & ((uint16)1 << 7)) && !haveViewSet)
		  {
			ViewSet(108);
			haveViewSet = TRUE;
		  }
		  //��ѹ
		  if ((statusViewNum & ((uint16)1 << 8)) && !haveViewSet)
		  {
			ViewSet(109);
			haveViewSet = TRUE;
		  }
		  //ȱ��
		  if ((statusViewNum & ((uint16)1 << 9)) && !haveViewSet)
		  {
			ViewSet(108);
			haveViewSet = TRUE;
		  }
		  //��λ��·
		  if ((statusViewNum & ((uint16)1 << 10)) && !haveViewSet)
		  {
			ViewSet(113);
			haveViewSet = TRUE;
		  }
		  //���̿�·
		  if ((statusViewNum & ((uint16)1 << 2)) && !haveViewSet && !temperatureCheckTime)
		  {
			ViewSet(103);
			haveViewSet = TRUE;
		  }
		  //IGBT1̽ͷ��·
		  if ((statusViewNum & ((uint16)1 << 4)) && !haveViewSet && !temperatureCheckTime)
		  {
			ViewSet(105);
			haveViewSet = TRUE;
		  }
		  //IGBT2̽ͷ��·
		  if ((statusViewNum & ((uint16)1 << 6)) && !haveViewSet && !temperatureCheckTime)
		  {
			ViewSet(105);
			haveViewSet = TRUE;
		  }
		//����̽ͷ��·
		if ((statusViewNum & ((uint16)1 << 11)) && !haveViewSet && !temperatureCheckTime)
		{
			ViewSet(107);
			haveViewSet = TRUE;
		}
                CLEAR_WD;
		if (rangeNow == 0)
		{
			//Ϊ0��λʱ
			firstOpen = FALSE;
			if (fanTime == 0)
			{
				fanTime = 1;//�ȴ��رշ��
			}
			if (!haveViewSet)
			{
				ViewSet(0);
			}
			fixPWM(0);
                        nullPot =0;//�޹�����
	                cTransformerCut = 0;//����״̬
                        //���ù���
                        statusViewNum &= ~((uint16)1 << 0);//�޹� ����
                        statusViewNum &= ~((uint16)1 << 1);//���̳�����0 ����
                        statusViewNum &= ~((uint16)1 << 3);//IGBT1������0 ����
                        statusViewNum &= ~((uint16)1 << 5);//IGBT2������0 ����
                        statusViewNum &= ~((uint16)1 << 12);//���׳�����0 ����
                        statusViewNum &= ~((uint16)1 << 13);//IGBT����������0 ����
                        statusViewNum &= ~((uint16)1 << 15);//���̲�ͨ�����������������0 ����
		}
		else
		{
			//���̳���
			if ((statusViewNum & ((uint16)1 << 1)) && !haveViewSet)
			{
				ViewSet(102);
				haveViewSet = TRUE;
			}
			//IGBT1����
			if ((statusViewNum & ((uint16)1 << 3)) && !haveViewSet)
			{
				ViewSet(104);
				haveViewSet = TRUE;
			}
			//IGBT2����
			if ((statusViewNum & ((uint16)1 << 5)) && !haveViewSet)
			{
				ViewSet(104);
				haveViewSet = TRUE;
			}
			//���׳���
			if ((statusViewNum & ((uint16)1 << 12)) && !haveViewSet)
			{
				ViewSet(106);
				haveViewSet = TRUE;
			}
                        //���̲�ͨ���������������
                        if((statusViewNum & ((uint16)1 << 15)) && !haveViewSet)
                        {                        
                            ViewSet(110);
                            haveViewSet = TRUE;
                            nullPot =0;//�޹�����
                        }
			//IGBT��������
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
                                  if(igbtErrorCheckTime == 0)//ʱ���������ڸ�λ���״̬
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
                                
                                nullPot =0;//�޹�����
	                        cTransformerCut = 0;//����״̬
			}
                        //�޹�
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
	                        cTransformerCut = 0;//����״̬
			}
			if (!haveViewSet)
			{
				if (firstOpen)
				{
					fixPWM(0);//�ر����
					ViewSet(0);//��ʾ0��λ
					if (fanTime == 0)
					{
						fanTime = 1;//�ȴ��رշ��
					}
                                        //0��������0
                                        nullPot =0;//�޹�����
	                                cTransformerCut = 0;//����״̬
				}
				else
				{
                                        if(Test_Bit(P0,6))
                                        {
                                          fixPWM(0);
                                          nullPot =0;//�޹�����
	                                  cTransformerCut = 0;//����״̬
                                        }
                                        else
                                        {
                                          fixPWM(rangeNow);//�������
                                        }
					ViewSet(rangeNow);//��ʾ��λ
					fanTime = 0;//�������   
				}
			}
			else
			{
                          	if (firstOpen || !checkTimeOn)
				{
				    fixPWM(0);//�ر����
                                    nullPot =0;//�޹�����
	                            cTransformerCut = 0;//����״̬
                                }
				if (fanTime == 0)
				{
					fanTime = 1;//�ȴ��رշ��
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
        uint4 temp = get_13ADC();// 1��ʾ�޹�
	if ((temp != 1) && !(statusViewNum & temp_2))
	{
		//��⵽�й�����ʾ�й�
		nullPot = 0;
		return;
	}
	if ((temp != 1) && (statusViewNum & temp_2))
	{
		//��⵽�й�����ʾ�޹�
                delay(2);
                if(get_13ADC()==1)
                  return;
		statusViewNum &= ~temp_2;//��0 ����
		nullPot = 0;
		return;
	}
	if ((temp == 1) && (statusViewNum & temp_2))
	{
		//��⵽�޹����޹�
                nullPot=0;
		return;
	}
	if ((temp == 1) && !(statusViewNum & temp_2))
	{
		//��⵽�޹����й�
                nullPot++;
		if (nullPot >=10)
		{
			nullPot = 0;
                        nullPotCheckTime =0;
                        nullPotLay =0;
                        checkNullPot =0;
			statusViewNum |= temp_2;//��1 �޹�״̬
		}
	}
}
#pragma inline=forced
void DetectCoilHot()
{
	uint16 temp_2 = (uint16)1 << 1;
	uint4 temp = get_07ADC();//1 �¸�
	if ((temp != 1) && !(statusViewNum & temp_2))
	{
		//����¶���������ʾ�¶�����
		return;
	}
	if ((temp != 1) && (statusViewNum & temp_2))
	{
		//����¶���������ʾ�¶Ȳ�����
		return;
	}
	if ((temp == 1) && (statusViewNum & temp_2))
	{
		//����¶Ȳ���������ʾ�¶Ȳ�����
		return;
	}
	if ((temp == 1) && !(statusViewNum & temp_2))
	{
		//����¶Ȳ���������ʾ�¶�����
		delay(2);//��ʱ2ms
		if (get_07ADC() != 1)
			return;
	        statusViewNum |= temp_2;//��1 ������
	}

}
#pragma inline=forced
void DetectCoilCut()
{
	uint16 temp_2 = (uint16)1 << 2;
	uint4 temp = get_07ADC();//2 ̽ͷ��·
	if ((temp != 2) && !(statusViewNum & temp_2))
	{
		//�����������ʾ����
		return;
	}
	if ((temp != 2) && (statusViewNum & temp_2)&& errorLay>=4)
	{
		//�����������ʾ������
		delay(2);//��ʱ2ms
		if (get_07ADC() == 2)
			return;
		statusViewNum &= ~temp_2;//��0 ����
		return;
	}
	if ((temp == 2) && (statusViewNum & temp_2))
	{
		//��鲻��������ʾ������
		return;
	}
	if ((temp == 2) && !(statusViewNum & temp_2))
	{
		//��鲻��������ʾ����
		delay(2);//��ʱ2s
		if (get_07ADC() != 2)
			return;
			statusViewNum |= temp_2;//��1 ������
                errorLay = 0;
	}
}
#pragma inline=forced
void DetectIGBTHot_1()
{
	uint16 temp_2 = (uint16)1 << 3;
	uint4 temp = get_04ADC();//2����
	if ((temp != 2) && !(statusViewNum & temp_2))
	{
		//����������
		return;
	}
	if ((temp != 2) && (statusViewNum & temp_2))
	{
		//�����Ҳ�����
		return;
	}
	if ((temp == 2) && (statusViewNum & temp_2))
	{
		//�������Ҳ�����
		return;
	}
	if ((temp == 2) && !(statusViewNum & temp_2))
	{
		//������������
		delay(2);
		if (get_04ADC() != 2)
			return;
			statusViewNum |= temp_2;//��1 ������
	}
}
#pragma inline=forced
void DetectIGBTCut_1()
{
	uint16 temp_2 = (uint16)1 << 4;
	uint4 temp = get_04ADC();//1��·
	if ((temp != 1) && !(statusViewNum & temp_2))
	{
		//����������
		return;
	}
	if ((temp != 1) && (statusViewNum & temp_2)&& errorLay>=4)
	{
		//�����Ҳ�����
		delay(2);
		if (get_04ADC() == 1)
			return;
		statusViewNum &= ~temp_2;//��0 ����
		return;
	}
	if ((temp == 1) && (statusViewNum & temp_2))
	{
		//�������Ҳ�����
		return;
	}
	if ((temp == 1) && !(statusViewNum & temp_2))
	{
		//������������
		delay(2);
		if (get_04ADC() != 1)
			return;
		statusViewNum |= temp_2;//��1 ������
                errorLay = 0;
	}
}
#pragma inline=forced
void DetectIGBTHot_2()
{
	uint16 temp_2 = (uint16)1 << 5;
	uint4 temp = get_11ADC();//2����
	if ((temp != 2) && !(statusViewNum & temp_2))
	{
		//����������
		return;
	}
	if ((temp != 2) && (statusViewNum & temp_2))
	{
		//�����Ҳ�����
		return;
	}
	if ((temp == 2) && (statusViewNum & temp_2))
	{
		//�������Ҳ�����
		return;
	}
	if ((temp == 2) && !(statusViewNum & temp_2))
	{
		//������������
		delay(2);
		if (get_11ADC() != 2)
			return;
		statusViewNum |= temp_2;//��1 ������
	}
}
#pragma inline=forced
void DetectIGBTCut_2()
{
	uint16 temp_2 = (uint16)1 << 6;
	uint4 temp = get_11ADC();//1��·
	if ((temp != 1) && !(statusViewNum & temp_2))
	{
		//����������
		return;
	}
	if ((temp != 1) && (statusViewNum & temp_2)&& errorLay>=4)
	{
		//�����Ҳ�����
		delay(2);
		if (get_11ADC() == 1)
			return;
		statusViewNum &= ~temp_2;//��0 ����
		return;
	}
	if ((temp == 1) && (statusViewNum & temp_2))
	{
		//�������Ҳ�����
		return;
	}
	if ((temp == 1) && !(statusViewNum & temp_2))
	{
		//������������
		delay(2);
		if (get_11ADC() != 1)
			return;
		statusViewNum |= temp_2;//��1 ������
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
		//��ѹ��
		temp = 1;
	}
	else if (statusViewNum & ((uint16)1 << 8))
	{
		//��ѹ��
		temp = 2;
	}
	temp = get_03ADC(temp);//2 ѹ��
	if ((temp != 1) && !(statusViewNum & temp_2))
	{
		//����������
		vLow = 0;
                vLowOut =0;
		return;
	}
	if ((temp != 1) && (statusViewNum & temp_2)&& errorLay>=4)
	{
		//�����Ҳ�����
                vLowOut++;
                if(vLowOut>4)
                {
		  statusViewNum &= ~temp_2;//��0 ����
                }
		vLow = 0;
		return;
	}
	if ((temp ==1) && (statusViewNum & temp_2))
	{
		//�������Ҳ�����
		vLow = 0;
                vLowOut =0;
		return;
	}
	if ((temp == 1) && !(statusViewNum & temp_2))
	{
		//������������
		vLow++;
		if (vLow >= 3)
		{
			vLow = 0;
			statusViewNum |= temp_2;//��1 ������
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
		//��ѹ��
		temp = 1;
	}
	else if ((statusViewNum & temp_2))
	{
		//��ѹ��
		temp = 2;
	}
	temp = get_03ADC(temp);//2 ѹ��
	if ((temp != 2) && !(statusViewNum & temp_2))
	{
		//����������
		vHight = 0;
                vHightOut =0;
		return;
	}
	if ((temp != 2) && (statusViewNum & temp_2)&& errorLay>=4)
	{
		//�����Ҳ�����
                vHightOut++;
                if(vHightOut>4)
                {
		  statusViewNum &= ~temp_2;//��0 ����
                }
		vHight = 0;
		return;
	}
	if ((temp == 2) && (statusViewNum & temp_2))
	{
		//�������Ҳ�����
		vHight = 0;
                vHightOut =0;
		return;
	}
	if ((temp == 2) && !(statusViewNum & temp_2))
	{
		//������������
		vHight++;
		if (vHight >= 3)
		{
			vHight = 0;
			statusViewNum |= temp_2;//��1 ������
		}
                vHightOut =0;
                errorLay = 0;
	}
}
#pragma inline=forced
void DetectSwitchCut()
{
	uint16 temp_2 = (uint16)1 << 10;
	uint4 temp = get_05ADC();//1 ��·
	if ((temp != 1) && !(statusViewNum & temp_2))
	{
		//����������
		return;
	}
	if ((temp != 1) && (statusViewNum & temp_2)&& errorLay>=4)
	{
		//�����Ҳ�����
		delay(2);
		if (get_05ADC() == 1)
			return;
		statusViewNum &= ~temp_2;//��0 ����
		return;
	}
	if ((temp == 1 )&& (statusViewNum & temp_2))
	{
		//�������Ҳ�����
		return;
	}
	if ((temp == 1) && !(statusViewNum & temp_2))
	{
		//������������
		delay(2);
		if (get_05ADC() != 1)
			return;
		statusViewNum |= temp_2;//��1 ������
                errorLay = 0;
	}
}
#pragma inline=forced
void DetectUnderPotCut()
{
	uint16 temp_2 = (uint16)1 << 11;
	uint4 temp = get_06ADC();//2 ����̽ͷ��·
	if ((temp != 2) && !(statusViewNum & temp_2))
	{
		//����������
		return;
	}
	if ((temp != 2) && (statusViewNum & temp_2) && errorLay>=4)
	{
		//�����Ҳ�����
		delay(2);
		if (get_06ADC() == 2)
			return;
		statusViewNum &= ~temp_2;//��0 ����
		return;
	}
	if ((temp == 2) && (statusViewNum & temp_2))
	{
		return;
	}
	if ((temp == 2) && !(statusViewNum & temp_2))
	{
		//������������
		delay(2);
		if (get_06ADC() != 2)
			return;
		statusViewNum |= temp_2;//��1 ������
                errorLay = 0;
	}
}
#pragma inline=forced
void DetectUnderPotHot()
{
	uint16 temp_2 = (uint16)1 << 12;
	uint4 temp = get_06ADC();//1����
	if ((temp != 1) && !(statusViewNum & temp_2))
	{
		//����������
		return;
	}
	if ((temp != 1) && (statusViewNum & temp_2))
	{
		//�����Ҳ�����
		return;
	}
	if ((temp == 1) && (statusViewNum & temp_2))
	{
		return;
	}
	if ((temp == 1 )&& !(statusViewNum & temp_2))
	{
		//������������
		delay(2);
		if (get_06ADC() != 1)
		return;
		statusViewNum |= temp_2;//��1 ������
	}
}
#pragma inline=forced
void DetectIgbtError()
{
	uint16 temp_2 = (uint16)1 << 13;
	uint4 temp = Test_Bit(P3, 3);//0������
	if (FALUT_OPEN && temp && !(statusViewNum & temp_2))
	{
		//����������
		return;
	}
	if (FALUT_OPEN && temp && (statusViewNum & temp_2))
	{
		//�����Ҳ�����
                if(nulligbtToLay>=4)
                {
                    statusViewNum &= ~temp_2;//��0 ����
                }
                else if(nulligbtToLay>=1)
                {
                  nulligbtToLay++;
                }
		return;
	}
	if ((!FALUT_OPEN || !temp)&& (statusViewNum & temp_2))
	{
		//�������Ҳ�����
		return;
	}
	if ((!FALUT_OPEN || !temp)&& !(statusViewNum & temp_2))
	{
		//������������
                statusViewNum |= temp_2;//��1 ������
                nulligbtLay =0;
                nulligbtToLay =0;//���ù��ϻָ�ʱ��
	}

}
#pragma inline=forced
void DetectTransformerCut()
{
	uint16 temp_2 = (uint16)1 << 15;
	uint16 temp = getADCNum(12);//0��ʾ���̶���
	if ((temp>=5)&& !PWMChange()&& !(statusViewNum & temp_2))
	{
		//����������
		cTransformerCut = 0;
		return;
	}
	if ((temp>=5)&& !PWMChange()&& (statusViewNum & temp_2))
	{
		//�����Ҳ�����
		//delay(2);
		//if (getADCNum(12)<5)
		//	return;
		//statusViewNum &= ~temp_2;//��0 ����
		cTransformerCut = 0;
		return;
	}
	if (((temp<5)|| PWMChange()) && (statusViewNum & temp_2))
	{
		//�������Ҳ�����
		//delay(2);
                cTransformerCut = 0;
		return;
	}
	if (((temp<5)|| PWMChange())&& !(statusViewNum & temp_2))
	{
		//������������
		cTransformerCut++;
		if (cTransformerCut >= 30)
		{
			cTransformerCut = 0;
			statusViewNum |= temp_2;//��1 ������
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
	if (ShowNum<100 && ShowNum>0)//�¶�ģʽ
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
		// tempurature =getTemperatureByAnum(6);//�����¶�
		//getADCNum(13) ���뻥�������� AD
		//getADCNum(12) ���������������С
		set_TM1629_Down(tempurature, 1);            
	}
	else//ʱ��ģʽ
	{
		set_TM1629_Down(0, 0);
                //set_TM1629_Down(getVo(), 1);
	}
	whileUpdate_TM1629();
}
#endif
//�ж�  __TA_OVERFLOW_vector
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
    //�ر����
    if(PWM_OPEN)//�ڿ���״̬
    {
      P03INTL &= 0xFC;//�ر��ж�
      closePWM();//�ر����
    }
    P03PND &= ~0x01;
  }
}
#pragma vector=__P34_EXTERNAL_INT_vector
__interrupt void P34INT_vector()
{
  if(P03PND & 0x02)
  {
    //��λ����
    if(PWM_OPEN)//�ڿ���״̬
    {
      P03INTL &= 0xF3;//�ر��ж� 
    }
    P03PND &= ~0x02;
  }
}
#pragma inline=forced
//A��ʱ��
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
			//INTERUPT ��������
#ifdef Screen_74HC164
			interuptUpdate_74HC164();
#elif defined Screen_TM1629
			interuptUpdate_TM1629();
#endif
                    if(PWM_OPEN && temperatureCheckTime)//ֻ�ڿ�ͨ״̬����¶���ת
                    {
                      temperatureCheckTime--;//��·��ʱ����ʱ
                    }
		}
	}
}
