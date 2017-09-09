#define _PWM_C_

#include "PWM.h"

volatile uint8 pwm =PWM_MIN;//

void initPWM()
{
  pwm=PWM_MIN;
}
void closePWM()
{
  AJ_OFF;
  if(TBCON != 0x73)
  {
    TBCON = 0x73;//关闭TIMER B
  }
  pwm = PWM_MIN;
  //TBDATAH = pwm;//pwm;//pwm/2 -1;
  //TBDATAL = pwm;//pwm;//pwm/2 -1;
  //P1CONL = 0xFC;
}
#pragma inline=forced
void openPWM()
{
  if(TBCON != 0x77)
  {
    TBCON = 0x77;//开启TIMER B
  }
  AJ_ON;

  //P1CONL = 0xFD;
}

void fixPWM(uint8 index)
{
    uint16 outCurrent = getADCNum(12);//输出互感器
    uint16 inCurrent = getADCNum(13);//输入互感器
    uint16 p=0;
    
      
    if( index !=0 && (getADCNum(4)<=0x0288 || getADCNum(11)<=0x0288))
    {
      index /= 2;
      if(index == 0)
      {
        index = 1;
      }
    }
    di;
    if(PWM_OPEN && (!FALUT_OPEN || !Test_Bit(P3, 3)))
    {
        closePWM();
        ei;
        return;
    }
    switch(index)
    {
      case 0:
        closePWM();
        P03INTL |= 0x01;//开启中断
        ei;
      return;
          case 1:
            p = PWM1;
      break;
          case 2:
            p = PWM2;
      break;
          case 3:
            p = PWM3;
      break;
          case 4:
            p = PWM4;
      break;
          case 5:
            p = PWM5;
      break;
          case 6:
            p = PWM6;
      break;
          case 7:
            p = PWM7;
      break;
          case 8:
            p = PWM8;
      break;
    }
    //if((inCurrent < RETURN_PWM) && (pwm > PWM_RETURN))
    //{
    //  --pwm;
    //}
    //else
    //{
    if(!FIX_OPEN)
    {
      pwm -= 1;
      P03INTL |= 0x04;//开启中断
    }
    else 
    {
      if(inCurrent !=0 && (outCurrent*4/inCurrent>=15))
      {
        --pwm;
      }
      else
      {
        if(outCurrent<(p-2))
        {
          ++pwm;
        }
        else if(outCurrent>(p+2))
        {
          --pwm;
        }
      }
    }
    //}
    pwm =Clamp(pwm,PWM_MIN,PWM_MAX);

    TBDATAH = pwm;//pwm;//pwm/2 -1;
    TBDATAL = pwm;//pwm;//pwm/2 -1;
    openPWM();
    ei;
}
//如果一直是低频率
uint4 PWMChange()
{
  return pwm <= PWM_MIN+2;
}
void testPotPwm()
{
    di;
    //pwm++;
    pwm =Clamp(PWM_POT,PWM_MIN,PWM_MAX);//Clamp(pwm,100,200);
    TBDATAH = pwm;//pwm;//pwm/2 -1;
    TBDATAL =pwm;//pwm;//pwm/2 -1;
    openPWM();
    ei;
}

void testPWM(uint8 index)
{
  di;
  if(index == 1)
  {
    pwm++;
    pwm =Clamp(pwm,PWM_MIN,PWM_MAX);
    TBDATAH = pwm;//pwm;//pwm/2 -1;
    TBDATAL =pwm;//pwm;//pwm/2 -1;
    openPWM();
  }
  else if(index == 2)
  {
    pwm--;
    pwm =Clamp(pwm,PWM_MIN,PWM_MAX);
    TBDATAH = pwm;//pwm;//pwm/2 -1;
    TBDATAL =pwm;//pwm;//pwm/2 -1;
    openPWM();
  }
  else
  {
    closePWM();
  }
  ei;
}
uint4 getPWMRate()
{
  uint16 range  = getADCNum(12);
  if(range>PWM8)
  {
    return 8;
  }
  else if(range>PWM7)
  {
    return 7;
  }
  else if(range>PWM6)
  {
    return 6;
  }
  else if(range>PWM5)
  {
    return 5;
  }
  else if(range>PWM4)
  {
    return 4;
  }
  else if(range>PWM3)
  {
    return 3;
  }
  else if(range>PWM2)
  {
    return 2;
  }
  else if(range>PWM1-5)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}
#pragma inline=forced
uint8 Clamp(uint8 num,uint8 mix,uint8 max)
{
  if(num<mix)
  {
    return mix;
  }
  else if(num >max)
  {
    return max;
  }
  return num;
}
