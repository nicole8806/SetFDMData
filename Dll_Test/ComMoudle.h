#ifndef ComMoudle_H
#define COmMoudle_H

#include <windows.h>


//打开串口标准接口
//int Open_Com();
extern "C" int __declspec(dllexport) Open_Comm();

//关闭串口标准接口
//int Open_Com();
extern "C" int __declspec(dllexport) Colse_Comm();
//写串口标准接口
//int Write_Comm(char *Write_Buffer,DWORD Write_Bytes);
int Write_Comm(HANDLE h_com ,char *Write_Buffer,DWORD Write_Bytes);
// 读串口标准接口
//int Read_Comm(char *Read_Comm,DWORD Read_Bytes);
int Read_Comm(HANDLE h_com, char *Read_Comm,DWORD Read_Bytes);




#endif