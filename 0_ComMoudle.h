#ifndef _COMMODULE_H_
#define _COMMODULE_H_

#include <windows.h>

/*******************************************************
1���ڽ�������API��������ǰ��һ��Ҫ�ȵ��ô˺���������ʼ��Ӳ��
2������ֵΪ��-1��ʱ����ʾ��ʼ��ʧ�ܣ�����������API��Ϊ�Ƿ�ֵ
����ֵΪ��1��ʱ����ʾ��ʼ���ɹ���Ȼ���ٵ�������������������
********************************************************/
int Open_Comm();

int Close_Comm();

//д���ڱ�׼�ӿ�
int Write_Comm(HANDLE h_com, UCHAR *Write_Buffer, DWORD Write_Bytes);

//�����ڱ�׼�ӿ�
int Read_Comm(HANDLE h_com, UCHAR *Read_Comm, DWORD Read_Bytes);

#endif	//_COMMODULE_H_