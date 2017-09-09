#define _74HC164_C_

#include "74HC164.h"

/**********************
  上升下降时间500ns
***********************/

//*****************************数码管显示**************************
__code const uint8 LED7Code[]={
        0xF7,//0 - 1111 0111
        0xBE,//1
        0xC4,//2
        0xA4,//3
        0xB2,//4
        0xA1,//5
        0x81,//6
        0xBC,//7
        0x80,//8
        0xB0,//9
        0x90,//a  1 001 0000
        0x83,//b 1 000 0011
        0xFF,//空白
        0xC9,//C 1100 1001
        0x86,//d 1000 0110
};

//灯状态 0=不亮 1=亮  2= 闪烁
enum LED_STATE redLedStatus;
enum LED_STATE blueLedStatus;
enum LED_STATE numLedStatus;
enum LED_STATE BuzzState;

uint8 screenNum;

// 0 亮  1不亮
uint4 blueState;
//0 是亮  1不亮
uint4 redState;
//0 是亮  1不亮
uint4 numState;

uint4 last74State;

/* up = 101 无锅
        102 线盘超温
        103 线盘开路
        104 IGBT超温
        105 IGBT开路
        106 锅底超温
        107 锅底开路
        108 电压低或缺相
        109 电压高
        110 
        111 线盘或者互感器损坏
        112 驱动故障
        113 档位开路
        200+KW 无锅
        KW     正常
        setRedState_74HC164(FLUSH);
        setBlueState_74HC164(FLUSH);
*/
void setNum_74HC164(uint8 num)
{
  
  switch (num) {
    case 101:
    BuzzState = OFF;
    numLedStatus = FLUSH;
    redLedStatus = FLUSH;
    blueLedStatus = OFF;
    screenNum = LED7Code[1];
    break;
    case 102:
    BuzzState = FLUSH;
    numLedStatus = FLUSH;
    redLedStatus = FLUSH;
    blueLedStatus = OFF;
    screenNum = LED7Code[2];
    break;
    case 103:
    BuzzState = FLUSH;
    numLedStatus = FLUSH;
    redLedStatus = FLUSH;
    blueLedStatus = OFF;
    screenNum = LED7Code[3];
    break;
    case 104:
    BuzzState = FLUSH;
    numLedStatus = FLUSH;
    redLedStatus = FLUSH;
    blueLedStatus = OFF;
    screenNum = LED7Code[4];
    break;
    case 105:
    BuzzState = FLUSH;
    numLedStatus = FLUSH;
    redLedStatus = FLUSH;
    blueLedStatus = OFF;
    screenNum = LED7Code[5];
    break;
    case 106:
    BuzzState = FLUSH;
    numLedStatus = FLUSH;
    redLedStatus = FLUSH;
    blueLedStatus = OFF;
    screenNum = LED7Code[6];
    break;
    case 107:
    BuzzState = FLUSH;
    numLedStatus = FLUSH;
    redLedStatus = FLUSH;
    blueLedStatus = OFF;
    screenNum = LED7Code[7];
    break;
    case 108:
    BuzzState = FLUSH;
    numLedStatus = FLUSH;
    redLedStatus = FLUSH;
    blueLedStatus = OFF;
    screenNum = LED7Code[8];
    break;
    case 109:
    BuzzState = FLUSH;
    numLedStatus = FLUSH;
    redLedStatus = FLUSH;
    blueLedStatus = OFF;
    screenNum = LED7Code[9];
    break;
    case 110:
    BuzzState = FLUSH;
    numLedStatus = FLUSH;
    redLedStatus = FLUSH;
    blueLedStatus = OFF;
    screenNum = LED7Code[10];
    break;
    case 111:
    BuzzState = FLUSH;
    numLedStatus = FLUSH;
    redLedStatus = FLUSH;
    blueLedStatus = OFF;
    screenNum = LED7Code[11];
    break;
    case 112:
    BuzzState = FLUSH;
    numLedStatus = FLUSH;
    redLedStatus = FLUSH;
    blueLedStatus = OFF;
    screenNum = LED7Code[13];
    break;
    case 113:
    BuzzState = FLUSH;
    numLedStatus = FLUSH;
    redLedStatus = FLUSH;
    blueLedStatus = OFF;
    screenNum = LED7Code[14];
    break;
    default:
    BuzzState = OFF;
    numLedStatus = ON;
    redLedStatus = num%100?ON:FLUSH;
    blueLedStatus = num%100?ON:OFF;
    screenNum = LED7Code[num%100];
    break;
  }
}

void init_74HC164()
{
  redLedStatus  = ON;
  blueLedStatus = ON;
  numLedStatus = ON;
  BuzzState = OFF;

  last74State =0;
  screenNum =0;
  blueState = 0xFF;

  COM_2_1;
  
  COM_1_1;
  blueState = 0;
  numState = 0;
  screenNum = LED7Code[8];
  display();
}
void whileUpdate_74HC164()
{
  if(redLedStatus == OFF)
  {
    COM_1_0;
  }
  else if(redLedStatus == ON)
  {
    COM_1_1;
  }
  else
  {
    redState?COM_1_0:COM_1_1;
  }
  
  if(blueLedStatus == OFF)
  {
    blueState = 1;
  }
  else if(blueLedStatus == ON)
  {
    blueState = 0;
  }
  
  if(numLedStatus == OFF)
  {
    numState = 1;
  }
  else if(numLedStatus == ON)
  {
    numState = 0;
  }
  di;
  display();
  ei;
}
void interuptUpdate_74HC164()
{
  if(redLedStatus == FLUSH)
  {
    redState = redState?0:1;
  }
  if(blueLedStatus == FLUSH)
  {// 现在blueState的状态 0x7F =亮  0xFF 不亮
      blueState = blueState?0:1;
  }
  if(numLedStatus == FLUSH)
  {
      if(numState == 1)//
      {
        numState = 0;//亮
        if(last74State ==0)
        {
          last74State =1;
          if(BuzzState == FLUSH)
          {
            BUZZ_ON;
          }
        }
      }
      else
      {
        numState = 1;
        last74State =0;
      }
   }
}

static void display()
{
    uint8 count;
    uint8 tmp;
    if(blueState)//绿灯不亮
    {
      if(numState)//数字不亮
      {
        tmp = LED7Code[12];
      }
      else
      {
        tmp = screenNum;
      }
    }
    else
    {
      if(numState)//数字不亮
      {
        tmp = LED7Code[12] & 0x7F;
      }
      else
      {
        tmp = screenNum & 0x7F;
      }
    }
    COM_2_0;
    //消隐
    for(count=0;count<8;count++)
    {
      if(tmp &(1<<7))
      {
          DAT_0;
      }
      else {
          DAT_1;
      }
      CLK_0;      //CLK =0
      CLK_1;      //CLK =1
      tmp<<=1;
    }
    COM_2_1;
}
