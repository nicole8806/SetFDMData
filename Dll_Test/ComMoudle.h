#ifndef ComMoudle_H
#define COmMoudle_H

#include <windows.h>


//�򿪴��ڱ�׼�ӿ�
//int Open_Com();
extern "C" int __declspec(dllexport) Open_Comm();

//�رմ��ڱ�׼�ӿ�
//int Open_Com();
extern "C" int __declspec(dllexport) Colse_Comm();
//д���ڱ�׼�ӿ�
//int Write_Comm(char *Write_Buffer,DWORD Write_Bytes);
int Write_Comm(HANDLE h_com ,char *Write_Buffer,DWORD Write_Bytes);
// �����ڱ�׼�ӿ�
//int Read_Comm(char *Read_Comm,DWORD Read_Bytes);
int Read_Comm(HANDLE h_com, char *Read_Comm,DWORD Read_Bytes);




#endif