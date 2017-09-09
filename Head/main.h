#ifndef _MAIN_H_
#define _MAIN_H_

#include "config.h"

#ifndef _MAIN_C_

#endif

void sysInit();
void ioInit();
void defaultValue();
void DetectNullPot();//�޹����
void DetectCoilHot();//���̳���
void DetectCoilCut();//����̽ͷ��·
void DetectIGBTHot_1();//IGBT����
void DetectIGBTCut_1();//IGBT̽ͷ��·
void DetectIGBTHot_2();//IGBT����
void DetectIGBTCut_2();//IGBT̽ͷ��·
void DetectVLow();//��ѹ���
void DetectVHight();//��ѹ���
void DetectVCut();//ȱ����
void DetectSwitchCut();//��λ���ؿ�·
void DetectUnderPotCut();//����̽ͷ��·
void DetectUnderPotHot();//���׳���
void DetectIgbtError();//IGBT��������
void DetectTransformer();//���������װ��
void DetectTransformerCut();//���̶��˻����������������
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
