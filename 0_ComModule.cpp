#include <stdio.h>
#include "ComMoudle.h"
#include "GetConfig.h"

HANDLE hCom1 = (HANDLE)-1, hCom2 = (HANDLE)-1;
extern map<string, string> m_mapConfig;

#define PAUD_RATE	38400
/*******************************************************
1，在进行其他API函数调用前，一定要先调用此函数，来初始化硬件
2，返回值为“-1”时，表示初始化失败，调用其他的API均为非法值
返回值为“1”时，表示初始化成功，然后再调用其他函数才有意义
********************************************************/
int Open_Comm()
{
	COMMTIMEOUTS	TimeOuts;
	DCB				dcb;

	Close_Comm();

	hCom1 = CreateFile(TEXT(m_mapConfig.find(PORT_COM1)->second.c_str()), //COM1口
		GENERIC_READ | GENERIC_WRITE, //允许读和写
		0, //独占方式
		NULL,
		OPEN_EXISTING, //打开而不是创建
		//FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED, //重叠方式
		0,
		NULL);

	if (hCom1 == (HANDLE)-1)
		return -1;

	SetupComm(hCom1, 1024, 1024); //输入缓冲区和输出缓冲区的大小都是100

	//设定读超时
	TimeOuts.ReadIntervalTimeout = MAXDWORD;
	TimeOuts.ReadTotalTimeoutMultiplier = 100;
	TimeOuts.ReadTotalTimeoutConstant = 500;
	//在读一次输入缓冲区的内容后读操作就立即返回，
	//而不管是否读入了要求的字符。

	//设定写超时
	TimeOuts.WriteTotalTimeoutMultiplier = 100;
	TimeOuts.WriteTotalTimeoutConstant = 500;
	SetCommTimeouts(hCom1, &TimeOuts); //设置超时

	GetCommState(hCom1, &dcb);
	dcb.BaudRate = PAUD_RATE; //波特率为9600
	dcb.ByteSize = 8; //每个字节有8位
	dcb.Parity = NOPARITY; //无奇偶校验位
	dcb.StopBits = ONESTOPBIT; //1个停止位
	SetCommState(hCom1, &dcb);

	PurgeComm(hCom1, PURGE_TXCLEAR | PURGE_RXCLEAR);

	hCom2 = CreateFile(TEXT(m_mapConfig.find(PORT_COM2)->second.c_str()),//COM1口
		GENERIC_READ | GENERIC_WRITE, //允许读和写
		0, //独占方式
		NULL,
		OPEN_EXISTING, //打开而不是创建
		//FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED, //重叠方式
		0,
		NULL);
	if (hCom2 == (HANDLE)-1)
		return -1;

	SetupComm(hCom2, 1024, 1024); //输入缓冲区和输出缓冲区的大小都是100

	//设定读超时
	TimeOuts.ReadIntervalTimeout = MAXDWORD;
	TimeOuts.ReadTotalTimeoutMultiplier = 100;
	TimeOuts.ReadTotalTimeoutConstant = 500;
	//设定写超时
	TimeOuts.WriteTotalTimeoutMultiplier = 100;
	TimeOuts.WriteTotalTimeoutConstant = 500;

	SetCommTimeouts(hCom2, &TimeOuts); //设置超时

	//DCB dcb;
	GetCommState(hCom2, &dcb);
	dcb.BaudRate = PAUD_RATE; //波特率为9600
	dcb.ByteSize = 8; //每个字节有8位
	dcb.Parity = NOPARITY; //无奇偶校验位
	dcb.StopBits = ONESTOPBIT; //1个停止位
	SetCommState(hCom2, &dcb);

	PurgeComm(hCom2, PURGE_TXCLEAR | PURGE_RXCLEAR);

	return 1;
}

//关闭串口
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

//写串口标准接口
int Write_Comm(HANDLE h_com, UCHAR *Write_Buffer, DWORD Write_Bytes)
{
	DWORD	w_Bytes = 0;
	BOOL	bWriteStat = FALSE;
	bWriteStat = WriteFile(h_com, Write_Buffer, Write_Bytes, &w_Bytes, NULL);

	return w_Bytes;
}

// 读串口标准接口
int Read_Comm(HANDLE h_com, UCHAR *Read_Comm, DWORD Read_Bytes)
{
	BOOL	bReadStat = FALSE;
	DWORD	RED_BYTE = 0;
	bReadStat = ReadFile(h_com, Read_Comm, Read_Bytes, &RED_BYTE, NULL);

	PurgeComm(h_com, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

	return RED_BYTE;
}


