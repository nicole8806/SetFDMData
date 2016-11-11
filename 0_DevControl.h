#ifndef _DEVCONTROL_H_
#define _DEVCONTROL_H_

#include "ComMoudle.h"

//读备份伞和脚踏开关状态
unsigned char Read_Protect();

//读压力数据,拉力传感
unsigned int Read_Presure(int channel);

//控制风扇转动
int control_Fan(int flag);

//控制电缸
int Send_Packeg_Udp_Data(double x_Po, double y_Po, double z_Po);

/*******************************************************
1，函数的调用的前提是调用Open_Comm()的返回值为“1”，串口打开成功
2, 函数返回表示读取到的编码器数据，取值为负表示反方向运动位置，为正表示正方向运动位置
3，参数Encoder_chanel取值范围为‘1’和‘2’，表示打开两个不同编码器通道
********************************************************/
unsigned int Read_Encoder(int Encoder_chanel);

BOOL Init();

BOOL StartLaJYJ();

BOOL StopLaJYJ();

BOOL StartFangJYJ();

BOOL StopFangJYJ();

#endif	//_DEVCONTROL_H_