#include <stdio.h>
#include "ComMoudle.h"
#include "GetConfig.h"

HANDLE hCom1 = (HANDLE)-1, hCom2 = (HANDLE)-1;
extern map<string, string> m_mapConfig;

#define PAUD_RATE	38400
/*******************************************************
1���ڽ�������API��������ǰ��һ��Ҫ�ȵ��ô˺���������ʼ��Ӳ��
2������ֵΪ��-1��ʱ����ʾ��ʼ��ʧ�ܣ�����������API��Ϊ�Ƿ�ֵ
����ֵΪ��1��ʱ����ʾ��ʼ���ɹ���Ȼ���ٵ�������������������
********************************************************/
int Open_Comm()
{
	COMMTIMEOUTS	TimeOuts;
	DCB				dcb;

	Close_Comm();

	hCom1 = CreateFile(TEXT(m_mapConfig.find(PORT_COM1)->second.c_str()), //COM1��
		GENERIC_READ | GENERIC_WRITE, //�������д
		0, //��ռ��ʽ
		NULL,
		OPEN_EXISTING, //�򿪶����Ǵ���
		//FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED, //�ص���ʽ
		0,
		NULL);

	if (hCom1 == (HANDLE)-1)
		return -1;

	SetupComm(hCom1, 1024, 1024); //���뻺����������������Ĵ�С����100

	//�趨����ʱ
	TimeOuts.ReadIntervalTimeout = MAXDWORD;
	TimeOuts.ReadTotalTimeoutMultiplier = 100;
	TimeOuts.ReadTotalTimeoutConstant = 500;
	//�ڶ�һ�����뻺���������ݺ���������������أ�
	//�������Ƿ������Ҫ����ַ���

	//�趨д��ʱ
	TimeOuts.WriteTotalTimeoutMultiplier = 100;
	TimeOuts.WriteTotalTimeoutConstant = 500;
	SetCommTimeouts(hCom1, &TimeOuts); //���ó�ʱ

	GetCommState(hCom1, &dcb);
	dcb.BaudRate = PAUD_RATE; //������Ϊ9600
	dcb.ByteSize = 8; //ÿ���ֽ���8λ
	dcb.Parity = NOPARITY; //����żУ��λ
	dcb.StopBits = ONESTOPBIT; //1��ֹͣλ
	SetCommState(hCom1, &dcb);

	PurgeComm(hCom1, PURGE_TXCLEAR | PURGE_RXCLEAR);

	hCom2 = CreateFile(TEXT(m_mapConfig.find(PORT_COM2)->second.c_str()),//COM1��
		GENERIC_READ | GENERIC_WRITE, //�������д
		0, //��ռ��ʽ
		NULL,
		OPEN_EXISTING, //�򿪶����Ǵ���
		//FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED, //�ص���ʽ
		0,
		NULL);
	if (hCom2 == (HANDLE)-1)
		return -1;

	SetupComm(hCom2, 1024, 1024); //���뻺����������������Ĵ�С����100

	//�趨����ʱ
	TimeOuts.ReadIntervalTimeout = MAXDWORD;
	TimeOuts.ReadTotalTimeoutMultiplier = 100;
	TimeOuts.ReadTotalTimeoutConstant = 500;
	//�趨д��ʱ
	TimeOuts.WriteTotalTimeoutMultiplier = 100;
	TimeOuts.WriteTotalTimeoutConstant = 500;

	SetCommTimeouts(hCom2, &TimeOuts); //���ó�ʱ

	//DCB dcb;
	GetCommState(hCom2, &dcb);
	dcb.BaudRate = PAUD_RATE; //������Ϊ9600
	dcb.ByteSize = 8; //ÿ���ֽ���8λ
	dcb.Parity = NOPARITY; //����żУ��λ
	dcb.StopBits = ONESTOPBIT; //1��ֹͣλ
	SetCommState(hCom2, &dcb);

	PurgeComm(hCom2, PURGE_TXCLEAR | PURGE_RXCLEAR);

	return 1;
}

//�رմ���
int Close_Comm()
{
	if (hCom1 != (HANDLE)-1)
	{
		CloseHandle(hCom1);
		hCom1 = (HANDLE)-1;
	}

	if (hCom2 != (HANDLE)-1)
	{
		CloseHandle(hCom2);
		hCom2 = (HANDLE)-1;
	}

	return 1;
}

//д���ڱ�׼�ӿ�
int Write_Comm(HANDLE h_com, UCHAR *Write_Buffer, DWORD Write_Bytes)
{
	DWORD	w_Bytes = 0;
	BOOL	bWriteStat = FALSE;
	bWriteStat = WriteFile(h_com, Write_Buffer, Write_Bytes, &w_Bytes, NULL);

	return w_Bytes;
}

// �����ڱ�׼�ӿ�
int Read_Comm(HANDLE h_com, UCHAR *Read_Comm, DWORD Read_Bytes)
{
	BOOL	bReadStat = FALSE;
	DWORD	RED_BYTE = 0;
	bReadStat = ReadFile(h_com, Read_Comm, Read_Bytes, &RED_BYTE, NULL);

	PurgeComm(h_com, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

	return RED_BYTE;
}


