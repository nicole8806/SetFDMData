#ifndef _COMMODULE_H_
#define _COMMODULE_H_

#include <windows.h>

/*******************************************************
1，在进行其他API函数调用前，一定要先调用此函数，来初始化硬件
2，返回值为“-1”时，表示初始化失败，调用其他的API均为非法值
返回值为“1”时，表示初始化成功，然后再调用其他函数才有意义
********************************************************/
int Open_Comm();

int Close_Comm();

//写串口标准接口
int Write_Comm(HANDLE h_com, UCHAR *Write_Buffer, DWORD Write_Bytes);

//读串口标准接口
int Read_Comm(HANDLE h_com, UCHAR *Read_Comm, DWORD Read_Bytes);

#endif	//_COMMODULE_H_