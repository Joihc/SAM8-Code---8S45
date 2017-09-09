#define _AT24C01_C_

#include "AT24C01.h"
/*
  �������ڲ�������ҳд�����ܿ�ҳ��Ҳ��ֻд�����ֽڣ���ָ���׵�ַ DataAddress
        д��һ�����߶��(24c01���9�ֽ�)
  SlaveAddress  Ҫд��Ĵ�����Ӳ����ַ1010 A2 A1 A0 R/W [A2:A0] ��AT24C01
                ��оƬӲ����ַ�� R/W�Ƕ�дѡ��λ��0Ϊд��1Ϊ�����⺯���ڲ�
                �Ѷ�R/W���˴����ⲿ�̶�Ϊ0/1 �����ǽӵ�
  DataAddress   Ҫд��Ĵ��浥Ԫ��ʼ��ַ
  pbuf          ָ�����ݻ�������ָ��
  Len           д�����ݵĳ���
  ����˵�� 0->successful  1->fault
*/
/*
  �ڲ�������ҳд��Ϊ��ҳ��д
*/
short  wrToRomPageA(uint8 SlaveAddress, uint8 DataAddress,uint8 __tiny *pbuf,uint8 Len)
{
  uint8 i=0;
  IIC_Start();                                        //��������
  if(IIC_SendByte(SlaveAddress & 0xfe)== 1) return 1; //д����Ѱ���Ӧ����
  if(IIC_SendByte(DataAddress)== 1)return 1;          //�Ѱ���Ӧ����
  for(i =0;i<Len;i++)
  {
    if(IIC_SendByte(*pbuf++) == 1)return 1;           //��Ƭ������һ�ֽ�����
  }
  IIC_Stop();                                         //��������
  return 0;
}
/*
  ���ֽ�д�룬������ic��ҳ���ٶ���(д��ʱ��Լ �ֽ���n*10ms)
*/
short  wrToRomA(uint8 SlaveAddress, uint8 DataAddress,uint8 __tiny *pbuf,uint8 Len)
{
  uint8 i;
  while(Len--)
  {
    if(wrToRomPageA(SlaveAddress,DataAddress++,pbuf++,0x01))  //д��һ�ֽ�
      return 1;                                               //���ֽ�д��ʧ��
    SDA_T;                                                  //SDA =1 ��æ����
    //����ѭ������һ��delay(10)���棬Ϊ�˲������߲�ͣ�ķ������ݲ�������
    //������ʱ1ms�ټ��IC�Ƿ�д�����
    for(i =0;i<10;i++)
    {
      delay(1);
      IIC_Start();                                           //��������
      if(IIC_SendByte(SlaveAddress & 0xfe)==0)break;
    }
  }
  return 0;
}
/*
  �ڲ�������ҳд��Ϊ��ҳ��д
*/
uint8 wrToRomPageB(uint8 SlaveAddress, uint8 DataAddress,uint8 __tiny *pbuf,uint8 Len)
{
  uint8 i=0;
  IIC_Start();                                          //��������
  if(IIC_SendByte(SlaveAddress & 0xfe)==1)return 0xff;  //ʧ�ܷ���0xff
  if(IIC_SendByte(DataAddress)==1)return 0xff;          //ʧ�ܷ���0xff
  for(i=0;i<Len;)
  {
    if(IIC_SendByte(*pbuf++) ==1)return 0xff;           //ʧ�ܷ���0xff
    i++;                                                //���i++�ŵ�for�ϣ�һ��break �˳�ѭ��������ɱ���++
    DataAddress++;
    if((DataAddress & 0x07)==0)break;                   //ҳԽ�磬24c01ÿҳ8�ֽڣ�ÿҳ��ʼ��ַ��3Ϊ000
  }
  IIC_Stop();                                           //��������
  return (Len-i);                                       //����(Len -i)�ֽ�û��д��
}
/*
  ���ֽ�д�룬����ҳ���С���ƣ��ٶȿ�
*/
short  wrToRomB(uint8 SlaveAddress, uint8 DataAddress,uint8 __tiny *pbuf,uint8 Len)
{
  uint8 temp = Len;
  do
  {
    temp = wrToRomPageB(SlaveAddress,DataAddress+(Len - temp),pbuf+(Len - temp),temp);
    if(temp == 0xff) return 1;                          //ʧ��
    delay(10);
  }while(temp);
  return 0;                                             //�ɹ�
}
/*
	������������������������DataAddress��ַ������ȡLen���ֽڵ�pbuf��
	SlaveAddress 	Ҫ��ȡ�Ĵ�����Ӳ����ַ1010 A2 A1 A0 R/W [A2:A0]��AT24C01
			��оƬӲ����ַ��R/W�Ƕ�дѡ��λ��0Ϊд��1Ϊ�����⺯���ڲ�
					�Ѷ�R/W���˴����ⲿ�̶�Ϊ0/1 �����ǽӵ�
	DataAddress 	Ҫ��ȡ�Ĵ洢��Ԫ��ʼ��ַ
	pbu		ָ�����ݻ�������ָ��
	Len		��ȡ���ݳ���
	0- successful 1- fault
*/
short  rdFromROM(uint8 SlaveAddress,uint8  DataAddress,uint8 __tiny *pbuf,uint8 Len)
{
	uint8 i=0;						//λ��ʼ
	IIC_Start();						//��������
	if(IIC_SendByte(SlaveAddress & 0xfe) == 1)return 1;	//д����Ѱ���Ӧ����
	if(IIC_SendByte(DataAddress) == 1)return 1;		//�Ѱ���Ӧ����
								//
	IIC_Start();						//������������
	if(IIC_SendByte(SlaveAddress | 0x01)== 1)return 1;	//��
	for(i=0;i<Len-1;i++)
	{
		*pbuf++=IIC_RecByte();				//����1�ֽ�
		IIC_Ack();					//Ӧ��0������������Ҫ��ȡ��һ�ֽ�
	}
	*pbuf = IIC_RecByte();					//�������1�ֽ�
	IIC_NoAck();						//Ӧ��1�������������ٶ�ȡ
	IIC_Stop();						//��������
	return 0;
}
