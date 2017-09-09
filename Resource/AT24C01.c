#define _AT24C01_C_

#include "AT24C01.h"
/*
  描述：内部函数，页写，不能跨页，也可只写单个字节，向指定首地址 DataAddress
        写入一个或者多个(24c01最多9字节)
  SlaveAddress  要写入的从器件硬件地址1010 A2 A1 A0 R/W [A2:A0] 是AT24C01
                的芯片硬件地址。 R/W是读写选择位，0为写，1为读。这函数内部
                已对R/W做了处理，外部固定为0/1 这里是接地
  DataAddress   要写入的储存单元开始地址
  pbuf          指向数据缓冲区的指针
  Len           写入数据的长度
  返回说明 0->successful  1->fault
*/
/*
  内部函数，页写，为跨页编写
*/
short  wrToRomPageA(uint8 SlaveAddress, uint8 DataAddress,uint8 __tiny *pbuf,uint8 Len)
{
  uint8 i=0;
  IIC_Start();                                        //启动总线
  if(IIC_SendByte(SlaveAddress & 0xfe)== 1) return 1; //写命令，已包含应答函数
  if(IIC_SendByte(DataAddress)== 1)return 1;          //已包含应答函数
  for(i =0;i<Len;i++)
  {
    if(IIC_SendByte(*pbuf++) == 1)return 1;           //单片机发送一字节数据
  }
  IIC_Stop();                                         //结束总线
  return 0;
}
/*
  多字节写入，不考虑ic分页。速度慢(写入时间约 字节数n*10ms)
*/
short  wrToRomA(uint8 SlaveAddress, uint8 DataAddress,uint8 __tiny *pbuf,uint8 Len)
{
  uint8 i;
  while(Len--)
  {
    if(wrToRomPageA(SlaveAddress,DataAddress++,pbuf++,0x01))  //写入一字节
      return 1;                                               //单字节写入失败
    SDA_T;                                                  //SDA =1 判忙处理
    //以下循环可用一句delay(10)代替，为了不让总线不停的发送数据产生干扰
    //所以延时1ms再检查IC是否写入完毕
    for(i =0;i<10;i++)
    {
      delay(1);
      IIC_Start();                                           //启动总线
      if(IIC_SendByte(SlaveAddress & 0xfe)==0)break;
    }
  }
  return 0;
}
/*
  内部函数，页写，为跨页编写
*/
uint8 wrToRomPageB(uint8 SlaveAddress, uint8 DataAddress,uint8 __tiny *pbuf,uint8 Len)
{
  uint8 i=0;
  IIC_Start();                                          //启动总线
  if(IIC_SendByte(SlaveAddress & 0xfe)==1)return 0xff;  //失败返回0xff
  if(IIC_SendByte(DataAddress)==1)return 0xff;          //失败返回0xff
  for(i=0;i<Len;)
  {
    if(IIC_SendByte(*pbuf++) ==1)return 0xff;           //失败返回0xff
    i++;                                                //如果i++放到for上，一旦break 退出循环不能完成本次++
    DataAddress++;
    if((DataAddress & 0x07)==0)break;                   //页越界，24c01每页8字节，每页起始地址低3为000
  }
  IIC_Stop();                                           //结束总线
  return (Len-i);                                       //还有(Len -i)字节没能写入
}
/*
  多字节写入，不受页面大小限制，速度快
*/
short  wrToRomB(uint8 SlaveAddress, uint8 DataAddress,uint8 __tiny *pbuf,uint8 Len)
{
  uint8 temp = Len;
  do
  {
    temp = wrToRomPageB(SlaveAddress,DataAddress+(Len - temp),pbuf+(Len - temp),temp);
    if(temp == 0xff) return 1;                          //失败
    delay(10);
  }while(temp);
  return 0;                                             //成功
}
/*
	功能描述：连续读操作，从DataAddress地址连续读取Len个字节到pbuf中
	SlaveAddress 	要读取的从器件硬件地址1010 A2 A1 A0 R/W [A2:A0]是AT24C01
			的芯片硬件地址。R/W是读写选择位，0为写，1为读。这函数内部
					已对R/W做了处理，外部固定为0/1 这里是接地
	DataAddress 	要读取的存储单元开始地址
	pbu		指向数据缓冲区的指针
	Len		读取数据长度
	0- successful 1- fault
*/
short  rdFromROM(uint8 SlaveAddress,uint8  DataAddress,uint8 __tiny *pbuf,uint8 Len)
{
	uint8 i=0;						//位开始
	IIC_Start();						//启动总线
	if(IIC_SendByte(SlaveAddress & 0xfe) == 1)return 1;	//写命令，已包含应答函数
	if(IIC_SendByte(DataAddress) == 1)return 1;		//已包含应答函数
								//
	IIC_Start();						//重新启动总线
	if(IIC_SendByte(SlaveAddress | 0x01)== 1)return 1;	//读
	for(i=0;i<Len-1;i++)
	{
		*pbuf++=IIC_RecByte();				//接受1字节
		IIC_Ack();					//应答0，告诉器件还要读取下一字节
	}
	*pbuf = IIC_RecByte();					//接受最后1字节
	IIC_NoAck();						//应答1，告诉器件不再读取
	IIC_Stop();						//结束总线
	return 0;
}
