#ifndef _MAIN_H_
#define _MAIN_H_

#include "config.h"

#ifndef _MAIN_C_

#endif

void sysInit();
void ioInit();
void defaultValue();
void DetectNullPot();//无锅检测
void DetectCoilHot();//线盘超温
void DetectCoilCut();//线盘探头开路
void DetectIGBTHot_1();//IGBT超温
void DetectIGBTCut_1();//IGBT探头开路
void DetectIGBTHot_2();//IGBT超温
void DetectIGBTCut_2();//IGBT探头开路
void DetectVLow();//低压检测
void DetectVHight();//高压检测
void DetectVCut();//缺相检测
void DetectSwitchCut();//档位开关开路
void DetectUnderPotCut();//锅底探头开路
void DetectUnderPotHot();//锅底超温
void DetectIgbtError();//IGBT驱动故障
void DetectTransformer();//输出互感器装反
void DetectTransformerCut();//线盘断了或者输出互感器坏了
void SwitchSet();
void ViewSet(uint8 ShowNum);

 void TAInterupt();
 void P33Interupt();
 uint4 P33interrptOpen();
 void P34Interupt();
 
__interrupt void TA_OVERFLOW_vector();
__interrupt void TA_MATCH_CAPTURE_vector();
__interrupt void P33INT_vector();
__interrupt void P34INT_vector();
#endif
