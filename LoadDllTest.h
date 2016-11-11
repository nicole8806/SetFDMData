#ifndef LOAD_DLL_TEST
#define LOAD_DLL_TEST

#include <stdio.h>
#include <windows.h>

//���Խӿ�
typedef int(*lpAddFun)(int, int); //�궨�庯��ָ������
typedef int(*lpDecFun)(int, int); //�궨�庯��ָ������

//��׿��ƽӿ�
typedef int(*lpSend_Packeg_Udp_Fun)(double, double, double); //�궨�庯��ָ������
typedef int(*lpCreat_UDP_Fun)(); //�궨�庯��ָ������


//���ư�ӿ�
typedef int(*lpOpen_Comm)(); //�궨�庯��ָ������Open_Comm
typedef int(*lpControl_JYJ)(int, int); //�궨�庯��ָ������Control_Juanyangji
typedef int(*lpControl_JYJ_info)(int); //�궨�庯��ָ������Control_Juanyangji
typedef int(*lpControl_FS)(int); //�궨�庯��ָ������Control_Fengshan
typedef unsigned char(*lpGet_Protect)(); //�궨�庯��ָ������Get_Protect
typedef int(*lpGet_Tension_Data)(int); //�궨�庯��ָ������Get_Tension_Data
typedef int(*lpGet_Pull_Data)(int); //�궨�庯��ָ������Get_Pull_Data

typedef struct tag_hd_opt{
	HINSTANCE hDll; //DLL��� 
	lpAddFun				addFun; //����ָ��
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