#ifndef _DEVCONTROL_H_
#define _DEVCONTROL_H_

#include "ComMoudle.h"

//������ɡ�ͽ�̤����״̬
unsigned char Read_Protect();

//��ѹ������,��������
unsigned int Read_Presure(int channel);

//���Ʒ���ת��
int control_Fan(int flag);

//���Ƶ��
int Send_Packeg_Udp_Data(double x_Po, double y_Po, double z_Po);

/*******************************************************
1�������ĵ��õ�ǰ���ǵ���Open_Comm()�ķ���ֵΪ��1�������ڴ򿪳ɹ�
2, �������ر�ʾ��ȡ���ı��������ݣ�ȡֵΪ����ʾ�������˶�λ�ã�Ϊ����ʾ�������˶�λ��
3������Encoder_chanelȡֵ��ΧΪ��1���͡�2������ʾ��������ͬ������ͨ��
********************************************************/
unsigned int Read_Encoder(int Encoder_chanel);

BOOL Init();

BOOL StartLaJYJ();

BOOL StopLaJYJ();

BOOL StartFangJYJ();

BOOL StopFangJYJ();

#endif	//_DEVCONTROL_H_