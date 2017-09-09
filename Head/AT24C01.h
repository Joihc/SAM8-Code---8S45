#ifndef _AT24C01_H_
#define _AT24C01_H_

#include "config.h"
#include "IIC.h"

#ifndef _AT24C01_C_

#endif

#define SLAVE_ADDRESS (0xA0)    //从机地址 格式：1010 ***0

short  wrToRomA(uint8 SlaveAddress, uint8 DataAddress,uint8 __tiny *pbuf,uint8 Len);//Slow Write
short  wrToRomB(uint8 SlaveAddress, uint8 DataAddress,uint8 __tiny *pbuf,uint8 Len);//Quickly Write
short  rdFromROM(uint8 SlaveAddress, uint8 DataAddress,uint8 __tiny *pbuf,uint8 Len);//Quickly Read
short  wrToRomPageA(uint8 SlaveAddress, uint8 DataAddress,uint8 __tiny *pbuf,uint8 Len);
uint8 wrToRomPageB(uint8 SlaveAddress, uint8 DataAddress,uint8 __tiny *pbuf,uint8 Len);

#endif /* _AT24C01_H_ */
