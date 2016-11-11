#ifndef LOAD_DLL_TEST
#define LOAD_DLL_TEST

#include <stdio.h>
#include <windows.h>

//测试接口
typedef int(*lpAddFun)(int, int); //宏定义函数指针类型
typedef int(*lpDecFun)(int, int); //宏定义函数指针类型

//电缸控制接口
typedef int(*lpSend_Packeg_Udp_Fun)(double, double, double); //宏定义函数指针类型
typedef int(*lpCreat_UDP_Fun)(); //宏定义函数指针类型


//控制板接口
typedef int(*lpOpen_Comm)(); //宏定义函数指针类型Open_Comm
typedef int(*lpControl_JYJ)(int, int); //宏定义函数指针类型Control_Juanyangji
typedef int(*lpControl_JYJ_info)(int); //宏定义函数指针类型Control_Juanyangji
typedef int(*lpControl_FS)(int); //宏定义函数指针类型Control_Fengshan
typedef unsigned char(*lpGet_Protect)(); //宏定义函数指针类型Get_Protect
typedef int(*lpGet_Tension_Data)(int); //宏定义函数指针类型Get_Tension_Data
typedef int(*lpGet_Pull_Data)(int); //宏定义函数指针类型Get_Pull_Data

typedef struct tag_hd_opt{
	HINSTANCE hDll; //DLL句柄 
	lpAddFun				addFun; //函数指针
	lpDecFun				decFun;
	lpSend_Packeg_Udp_Fun	Send_Packeg_Udp;
	lpCreat_UDP_Fun			Creat_UDP_Fun;
	lpOpen_Comm				open_com;
	lpControl_JYJ			control_JYJ;
	lpControl_JYJ_info		control_JYJ_info;
	lpControl_FS			control_FS;
	lpGet_Protect			get_protec;
	lpGet_Tension_Data		get_tension_data;
	lpGet_Pull_Data			get_pull_data;
};

bool LoadDllTest();

#endif