/**********************************************************
*Author: TY
*http://www.fg-china.net   
**********************************************************/

#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <iostream>
#include "FGNetData.h"
#include "GetConfig.h"
#include "Dll_Test/ComMoudle.h"
#include "Dll_Test/Center_Control.h"
#include "Dll_Test/DLL_Test.h"



using namespace std;

//��̤�忪��
#define FOOT_BOARD_SWITCH		(1<<5)		//��̤���ڵ�5λ����0��ʼ

//����ɡ����
#define PARACHUTE_SWITCH		(1<<7)		//����ɡ�����ڵ�7λ����0��ʼ

//���ȿ���
#define FS_OPEN		1
#define FS_CLOSE	0

//����(���ݰ�)����
#define RIGHT_FRONT_DRAG		1	//��ǰ����
#define RIGHT_AFTER_DRAG		2	//�Һ�����
#define LEFT_AFTER_DRAG			3	//�������
#define LEFT_FRONT_DRAG			4	//��ǰ����

//���ݴ�

//#define SLN_1
//#define SLN_2
#define SLN_3

//���������
#define JYJ_STOP			0
#define JYJ_FANG			1
#define JYJ_LA				2

#ifdef SLN_1
#define JYJ_LAFANG_TIME		3500	//ms
#endif

#ifdef SLN_3
#define JYJ_LAFANG_TIME		3500	//ms
#endif

//��ײ���
#define MIN_DG_HEIGHT			0.0
#define MAX_DG_HEIGHT			400.0



//����1
#ifdef SLN_1
#define INIT_DG_HEIGHT			200.0		//��ʼ����׸߶�
#define LAND_DG_HEIGHT			100.0		//��½ʱ�ĵ�׸߶�
#define OPEN_CHUTE_DG_HEIGHT	350.0		//��ɡ˲�佫��������ĸ߶�
#endif

//����2
#ifdef SLN_2
#define INIT_DG_HEIGHT			0.0		//��ʼ����׸߶�
#define JYJ_LA_TIME				
#define JYJ_FANG_TIME				
#define OPEN_CHUTE_DG_HEIGHT	400.0		//��ɡ˲�佫��������ĸ߶�
#endif

//����1
#ifdef SLN_3
#define INIT_DG_HEIGHT			200.0		//��ʼ����׸߶�
#define LAND_DG_HEIGHT			100.0		//��½ʱ�ĵ�׸߶�
#define OPEN_CHUTE_DG_HEIGHT	400.0		//��ɡ˲�佫��������ĸ߶�
#endif


//����
#define LAND_HEIGHT				164			//��½�߶�164Ӣ�磬����Լ5��
#define OPEN_CHUTE_TIME			4000		//��ȫ��ɡʱ�䣬4000ms����4s
#define MAX_POS 400.0
#define MIN_POS 300.0

map<string, string> m_mapConfig;

FGNetData			gFgbuf;
SOCKADDR_IN			addrFgSrv;		//Fg����˵�ַ��Ϣ
//extern SOCKADDR_IN	addrDgSrv;		//��׷���˵�ַ��Ϣ

static int nDebug = 0;
static BOOL bRecvThreadExit = FALSE;
static BOOL	bOptDgTreadExit = FALSE;

//static int gRrightDrag = 0;
//static int gLeftDrag = 0;

void SleepTime(_In_ DWORD dwMilliseconds)
{
	DWORD		dwTime = GetTickCount();
	while ((GetTickCount() - dwTime) < dwMilliseconds)
	{
		Sleep(20);	//��֤���ᳬ��20ms
	}	
}

//��ʼ���豸
BOOL InitDev()
{
	//cout << "Initializing Device......" << endl;
	cout << "���ڳ�ʼ��Ӳ��..." << endl;

	if (nDebug)
		goto end;

	init_hardware();

	//1. ��׸�λ,�˴��Ƿ���ȷ��Ҫ������
	Creat_UDP();
	Send_Packeg_Udp_Data(INIT_DG_HEIGHT, INIT_DG_HEIGHT, INIT_DG_HEIGHT);


	//2. �������λ
	Control_Juanyangji_info(JYJ_LA);
#if defined(SLN_1)
	SleepTime(JYJ_LAFANG_TIME);
#endif

#if defined(SLN_2)
	SleepTime(7000);
#endif

#if defined(SLN_3)
	SleepTime(JYJ_LAFANG_TIME-300);
#endif
	//SleepTime(JYJ_LAFANG_TIME + 1000);
	//SleepTime(9100);
	Control_Juanyangji_info(JYJ_STOP);
end:
	cout << "Ӳ����λ��ɣ�����ɡԱ����ɡ�ߣ�����3D�۾����Ⱥý�̤�壬�ȴ�����׼����ɡ..." << endl;

	return TRUE;
}

//���ݲ���״̬���Ʒ����˶�
BOOL OpenFanByDoorStatus()
{
	FGNetData	fgbuf_tmp = gFgbuf;

	//cout << "Waiting to open door......" << endl;
	cout << "������..." << endl;
	while (fgbuf_tmp.mp_osi[2] != 10)	//����û�򿪣�һֱ�ȴ�
	{
		fgbuf_tmp = gFgbuf;
		Sleep(50);
	}

	//cout << "Door is opend, now open the fan......" << endl;
	if (!nDebug)
		Control_Fengshan(FS_OPEN);	//���Ʒ��ȴ���//Ӳ������

	return TRUE;
}

//��ʼ��������Fg����������Ϣ
BOOL StartJump()
{
	UCHAR		cPress_down = 0;
	FGNetData	fgbuf_tmp;

	cout << "�����Ѵ򿪣�����..." << endl;

	//�ȼ���̤���Ƿ���£�����ȡ��̤���ֵ�Ƿ�Ϊ0
	do
	{
		if (nDebug)
			cPress_down = 0x00;
		else
			cPress_down = Get_Protect();	//��ȡ��̤��״̬��Ӳ������

		if (!(cPress_down & FOOT_BOARD_SWITCH))
			break;

	} while (TRUE);

	//�ټ���̤���Ƿ��ɿ�������ȡ��̤���ֵ�Ƿ�Ϊ1
	do
	{
		if (nDebug)
			cPress_down = FOOT_BOARD_SWITCH;
		else
			cPress_down = Get_Protect();	//��ȡ��̤��״̬��Ӳ������

		if (cPress_down & FOOT_BOARD_SWITCH)
		{
			fgbuf_tmp = gFgbuf;
			fgbuf_tmp.oil_px[3] = 10;
			fgbuf_tmp.ConverData();
			SOCKET	sockClient = socket(AF_INET, SOCK_DGRAM, 0);
			sendto(sockClient, (char*)(&fgbuf_tmp), sizeof(fgbuf_tmp), 0, (SOCKADDR*)&addrFgSrv, sizeof(SOCKADDR));
			closesocket(sockClient);

			//Send_Packeg_Udp_Data(100.0, 100.0, 100.0);	//��ɡ֮�����ʧ�صĸо�

			//cout << "Jump success......" << endl;
			break;
		}
	} while (TRUE);

	return TRUE;
}

//����ɡ
BOOL OpenUmbrella()
{
	FGNetData	fgbuf_tmp = gFgbuf;
	UCHAR		cPress_down = 0;
	DWORD		dwJumpTime;
	DWORD		dwTimeInternal = 0;
	BOOL		bOpenFail = FALSE;

	//cout << "Waiting to open main umbrella......" << endl;
	if (!nDebug)
	{
		//StartFangJYJ();
		//Control_Juanyangji_info(JYJ_FANG);
		//Send_Packeg_Udp_Data(100.0, 100.0, 100.0);	//�ӳ�ʼ��200.0�任��100.0
	}

	dwJumpTime = GetTickCount();

	cout << "���ڴ���ɡ..." << endl;
	while (fgbuf_tmp.mp_osi[3] != 10)	//��ɡû�򿪣�һֱ�ȴ�
	{
		fgbuf_tmp = gFgbuf;
		Sleep(50);

		dwTimeInternal = GetTickCount() - dwJumpTime;
		//4sʱ�䣬��ɡû�д򿪣���ɡ��ʼ��ɡ����ɡ��ʱ��Ϊ4s,��4s��û�д���ɡ������Ϊ��ɡ��ʧ�ܣ�Ӧ��ȥ�򿪱���ɡ
		if (dwTimeInternal >= OPEN_CHUTE_TIME)
		{
			bOpenFail = TRUE;
			break;
		}
	}

	if (bOpenFail)
	{
		//cout << "Open main umbrella fail, waiting to open backup umbrella......" << endl;
		cout << "��ɡû�д򿪣���򿪱���ɡ..." << endl;
		do
		{
			if (nDebug)
				cPress_down = PARACHUTE_SWITCH;
			else
				cPress_down = Get_Protect();

			if (cPress_down & PARACHUTE_SWITCH)
			{
				fgbuf_tmp = gFgbuf;
				fgbuf_tmp.fuel_px[3] = 10;
				fgbuf_tmp.ConverData();
				SOCKET	sockClient = socket(AF_INET, SOCK_DGRAM, 0);
				sendto(sockClient, (char*)(&fgbuf_tmp), sizeof(fgbuf_tmp), 0, (SOCKADDR*)&addrFgSrv, sizeof(SOCKADDR));
				closesocket(sockClient);
				//cout << "Open backup umbrella success......" << endl;
				cout << "����ɡ�Ѵ�..." << endl;
				break;
			}

			if (fgbuf_tmp.mp_osi[1] <= (fgbuf_tmp.fuel_px[3] + LAND_HEIGHT))	//�����߶Ȳ���5m������Ҫ��ɡ��ֱ����½
			{
				cout << "����ɡû�д򿪣��˴���ɡģ��ʧ�ܣ�ģ����Ա�����Ѿ�����..." << endl;
				cout << "��ʼ��½..." << endl;
				return FALSE;
			}

		} while (TRUE);
	}
	else
	{
#if defined(SLN_1)
		SleepTime(4000);	//ʹ�۾��еĿ�ɡ��ʵ�ʵĿ�ɡ����һ��
#endif

#if defined(SLN_3)
		SleepTime(1000);
		if (!nDebug)
		{
			Control_Juanyangji_info(JYJ_FANG);
			SleepTime(2000);
			Control_Juanyangji_info(JYJ_STOP);
			Creat_UDP();
			Send_Packeg_Udp_Data(100.0, 100.0, 100.0);	//�ӳ�ʼ��200.0�任��100.0
			SleepTime(1000);
		}
#endif

		if (!nDebug)
		{
			//dwTimeInternal = GetTickCount() - dwJumpTime;
			//StopFangJYJ();
			//Control_Juanyangji_info(STOP);
		}

		//Sleep(1500);
		//cout << "11111..." << endl;
		
		/*fgbuf_tmp = gFgbuf;
		fgbuf_tmp.oil_px[2] = -1;

		cout << "fgbuf_tmp.mp_osi_11: " << fgbuf_tmp.mp_osi[1] << endl;
		fgbuf_tmp.ConverData();
		SOCKET	sockClient = socket(AF_INET, SOCK_DGRAM, 0);
		sendto(sockClient, (char*)(&fgbuf_tmp), sizeof(fgbuf_tmp), 0, (SOCKADDR*)&addrFgSrv, sizeof(SOCKADDR));
		Sleep(1000);
		fgbuf_tmp = gFgbuf;
		cout << "fgbuf_tmp.mp_osi_22: " << fgbuf_tmp.mp_osi[1] << endl;
		Sleep(1000);
		fgbuf_tmp = gFgbuf;
		cout << "fgbuf_tmp.mp_osi_23: " << fgbuf_tmp.mp_osi[1] << endl;
		Sleep(1000);
		fgbuf_tmp = gFgbuf;
		cout << "fgbuf_tmp.mp_osi_33: " << fgbuf_tmp.mp_osi[1] << endl;
		fgbuf_tmp.oil_px[2] = 1;
		fgbuf_tmp.ConverData();
		sendto(sockClient, (char*)(&fgbuf_tmp), sizeof(fgbuf_tmp), 0, (SOCKADDR*)&addrFgSrv, sizeof(SOCKADDR));
		closesocket(sockClient);*/
		//cout << "Open main umbrella success......" << endl;
		cout << "��ɡ�Ѵ�..." << endl;
	}

	if (!nDebug)
	{
		Control_Juanyangji_info(JYJ_LA);
		Creat_UDP();
		Send_Packeg_Udp_Data(OPEN_CHUTE_DG_HEIGHT, OPEN_CHUTE_DG_HEIGHT, OPEN_CHUTE_DG_HEIGHT);	//�����������ģ����ק���, Ӳ������
		SleepTime(1000);
		Control_Juanyangji_info(JYJ_STOP);
		SleepTime(3000);

		//Sleep(100);
		//StartLaJYJ();
		//Control_Juanyangji_info(LA);
		//SleepTime(dwTimeInternal /*+ 500*/);	//��Ϊ�˵�����������ʱ��Ӧ��Ҫ�ȷŵ�ʱ�䳤
		//SleepTime(1000);
		//StopLaJYJ();
		//Control_Juanyangji_info(STOP);
	}

	return TRUE;
}

DWORD WINAPI ShakeDg(LPVOID lpThreadParameter)
{
	while (!bOptDgTreadExit)
	{
	}
}

//������½��λ�ú��½����ٶ�,������
BOOL ControlLandingPos()
{
	//�ж������ľ���
	BOOL	bFlag = FALSE;
	BOOL	bLandFlag = FALSE;
	BOOL	bFlag1 = FALSE, bFlag2 = FALSE, bFlag3 = FALSE, bFlag4 = FALSE;
	int		nLeftFrontDrag = 0, nLeftAfterDrag = 0, nRightFrontDrag = 0, nRightAfterDrag = 0;
	int		nLeftFrontInit = 0, nLeftAfterInit = 0, nRightFrontInit = 0, nRightAfterInit = 0;
	int		nShift = 0, s_nShift = 1, nShift_1 = 0, nShift_2 = 0, nShift_3 = 0, nShift_4 = 0;
	FGNetData	fgbuf_tmp;
	SOCKET		sockClient = socket(AF_INET, SOCK_DGRAM, 0);
	int		count1 = 0, count2 = 0;
	DWORD	dwThreadId = 0;
	HANDLE	hThread;
	

	//cout << "Now is falling, find landing position by operating left and right drag..." << endl;
	cout << "�����½����������ݴ�����ݰ���������½��λ�ú��½����ٶ�..." << endl;

	hThread = ::CreateThread(NULL, 0, ShakeDg, NULL, 0, &dwThreadId);

	nLeftFrontInit = Get_Pull_Data(LEFT_FRONT_DRAG);
	nLeftAfterInit = Get_Pull_Data(LEFT_AFTER_DRAG);
	nRightFrontInit = Get_Pull_Data(RIGHT_FRONT_DRAG);
	nRightAfterInit = Get_Pull_Data(RIGHT_AFTER_DRAG);

	do{
		SleepTime(300);

		if (nDebug)
		{
			nRightAfterDrag = 65002;
			nLeftAfterDrag = 65000;
		}
		else
		{
			nLeftFrontDrag = Get_Pull_Data(LEFT_FRONT_DRAG) - nLeftFrontInit;
			nLeftAfterDrag = Get_Pull_Data(LEFT_AFTER_DRAG) - nLeftAfterInit;
			nRightFrontDrag = Get_Pull_Data(RIGHT_FRONT_DRAG) - nRightFrontInit;
			nRightAfterDrag = Get_Pull_Data(RIGHT_AFTER_DRAG) - nRightAfterInit;
		}
		
		if (nRightAfterDrag > 120000)
			nRightAfterDrag = 65000;

		if (nLeftAfterDrag > 120000)
			nLeftAfterDrag = 65000;

		cout << "nLeftFrontDrag: " << nLeftFrontDrag << endl;
		cout << "nLeftAfterDrag: " << nLeftAfterDrag << endl;
		cout << "nRightFrontDrag: " << nRightFrontDrag << endl;
		cout << "nRightAfterDrag: " << nRightAfterDrag << endl;

		nShift_1 = nRightFrontDrag - nLeftFrontDrag;
		nShift_2 = nRightAfterDrag - nLeftAfterDrag;
		nShift_3 = min(nLeftAfterDrag, nRightFrontDrag) * (-2);
		nShift_4 = min(nLeftFrontDrag, nRightAfterDrag) * 2;

		//����Ϊ0
		if (abs(nShift_1) < 100)
			nShift_1 = 0;

		if (abs(nShift_2) < 100)
			nShift_2 = 0;

		if (abs(nShift_3) < 100)
			nShift_3 = 0;

		if (abs(nShift_4) < 100)
			nShift_4 = 0;

		nShift = nShift_1 + nShift_2 + nShift_3 + nShift_4;
		
		fgbuf_tmp = gFgbuf;

		/*10�׵�ʱ����ʾ������½*/
		if (!bLandFlag && (fgbuf_tmp.mp_osi[1] <= (fgbuf_tmp.fuel_px[3] + LAND_HEIGHT)))
		{
			cout << "������½����׼��..." << endl;
			Send_Packeg_Udp_Data(MAX_POS, MAX_POS, MAX_POS);
			bLandFlag = TRUE;
		}

		//�����߶Ȳ���5m������Ҫ��ɡ��ֱ����½
		//�˴����뵥λ��Ӣ�磬�ò����������Ǻ��Σ������ɽ�غ�����fgbuf_tmp.fuel_px[3]Ӣ�磬5m��Ϊ164Ӣ�硣
		if (fgbuf_tmp.mp_osi[1] <= (fgbuf_tmp.fuel_px[3] + /*LAND_HEIGHT*/82))
		{
			//��ʼ��½��ֹͣ��������
			fgbuf_tmp.oil_px[0] = (float)0.0;
			fgbuf_tmp.oil_temp[3] = (float)0.0;
			fgbuf_tmp.ConverData();
			sendto(sockClient, (char*)(&fgbuf_tmp), sizeof(fgbuf_tmp), 0, (SOCKADDR*)&addrFgSrv, sizeof(SOCKADDR));

			Sleep(4500);
			break;
		}

		cout << "fgbuf_tmp.mp_osi[1]: " << fgbuf_tmp.mp_osi[1] << endl;
		cout << "fgbuf_tmp.fuel_px[3]: " << fgbuf_tmp.fuel_px[3] << endl;

		if (!bFlag4 /*&& (count1 % 2 == 0)*/)
		{
			count2++;
			switch (count2)
			{
			case 1:
				Send_Packeg_Udp_Data(MIN_POS, MIN_POS, MAX_POS);
				break;
			case 2:
				Send_Packeg_Udp_Data(MIN_POS-100.0, MIN_POS-100.0, MAX_POS);
				//Send_Packeg_Udp_Data(MAX_POS, MIN_POS, MIN_POS);
				break;
			case 3:
				//Send_Packeg_Udp_Data(MIN_POS, MAX_POS, MIN_POS);
				Send_Packeg_Udp_Data(MIN_POS, MIN_POS, MAX_POS-100.0);
				break;
			case 4:
				//Send_Packeg_Udp_Data(MIN_POS, MIN_POS, MAX_POS);
				Send_Packeg_Udp_Data(MIN_POS+100.0, MIN_POS+100.0, MAX_POS - 100.0);
				break;
			case 5:
				Send_Packeg_Udp_Data(MAX_POS, MAX_POS, MAX_POS);
				break;
			}
			if (count2 == 5)
			{
				bFlag4 = TRUE;
				count1 = 0;
				count2 = 0;
			}
		}

		count1++;

		/*if (nShift != s_nShift)
			s_nShift = nShift;
		else
			continue;*/
		//����ת��
		fgbuf_tmp.oil_px[0] = nShift;	//����������
		//ˮƽ�ٶ�
		fgbuf_tmp.oil_px[1] = 6000 - min(min(nRightAfterDrag, nLeftAfterDrag) / 16820.0, 1.0) * 6000 \
								+ min(min(nRightFrontDrag, nLeftFrontDrag) / 16820.0, 1.0) * 6000;	//��λ��mm/s	
		
		//���¶���
		//fgbuf_tmp.oil_px[2] = 3;
		//���Ҷ���
		/*if (bFlag1)
		{
			fgbuf_tmp.oil_temp[3] = -0.12;
			bFlag1 = FALSE;
		}
		else
		{
			fgbuf_tmp.oil_temp[3] = 0.12;
			bFlag1 = TRUE;
		}*/

		fgbuf_tmp.ConverData();
			
		sendto(sockClient, (char*)(&fgbuf_tmp), sizeof(fgbuf_tmp), 0, (SOCKADDR*)&addrFgSrv, sizeof(SOCKADDR));

	} while (TRUE);
	
	closesocket(sockClient);

	return TRUE;
}

//��½
BOOL Landing()
{
	//cout << "Falling is over, Now is Landing..." << endl;
	//1. �رշ���
	if (!nDebug)
	{
		Control_Fengshan(FS_CLOSE);

#if defined(SLN_1)
		Send_Packeg_Udp_Data(LAND_DG_HEIGHT, LAND_DG_HEIGHT, LAND_DG_HEIGHT);
#endif

		//3. ���ƾ������������½����ɡ������ɡ�
		Control_Juanyangji_info(JYJ_FANG);
#if defined(SLN_1)
		SleepTime(JYJ_LAFANG_TIME);
#elif defined(SLN_2)
		SleepTime(7000);
#elif defined(SLN_3)
		Send_Packeg_Udp_Data(LAND_DG_HEIGHT, LAND_DG_HEIGHT, LAND_DG_HEIGHT);
		SleepTime(2300);
#endif
		Control_Juanyangji_info(JYJ_STOP);
	}
	
	//cout << "Landing is over..." << endl;
	cout << "��½��ϣ�����Աж��ɡ�߲��볡..." << endl;

	return TRUE;
}

//��fgfs���������߳���ں���
DWORD WINAPI RecvFromFgfs(LPVOID lpParam)
{
	FGNetData *pFgfuf = (FGNetData*)lpParam;

	//1. �����׽��ֲ���IP��ַ�Ͷ˿ں�
	SOCKET sockSrv = socket(AF_INET, SOCK_DGRAM, 0);	//UDP
	SOCKADDR_IN addrSrv;
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrSrv.sin_port = htons(atoi(m_mapConfig.find(PORT_APP2FG)->second.c_str()));	//�˿�5300
	bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));

	//2. ��������
	SOCKADDR_IN addrClient;
	FGNetData	fgbuf_tmp;
	int			len = sizeof(SOCKADDR);
	int			err = 0;

	while (!bRecvThreadExit)
	{
		memset(&fgbuf_tmp, 0, sizeof(fgbuf_tmp));
		err = recvfrom(sockSrv, (char *)(&fgbuf_tmp), sizeof(fgbuf_tmp), 0, (SOCKADDR*)&addrClient, &len);
		if (err != -1)
		{
			fgbuf_tmp.RecvConverData();
			memcpy(pFgfuf, &fgbuf_tmp, sizeof(FGNetData));
		}

		//cout << "��������" << endl;
		Sleep(100);
	}

	//cout << "ֹͣ����" << endl;
	return 0;
}

int main(int argc, char *argv[])
{
	int	nRet = -1;
	TCHAR szModulePath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szModulePath, MAX_PATH);
	(_tcsrchr(szModulePath, _T('\\')))[1] = '\0';
	strcat_s(szModulePath, CONFIG_FILE);

	//1. ��ȡ�����ļ�
	string s_filePath = szModulePath;
	ReadConfig(s_filePath, m_mapConfig);
	nDebug = atoi(m_mapConfig.find(DEBUG_NO_HARDWARE)->second.c_str());
	//cout << nDebug << endl;

	//2. ������Ҫ�õ�socketͨѶ�������ȳ�ʼ��socket�汾
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
		return -1;
	}
	//Fg����˵ĵ�ַ��Ϣ
	addrFgSrv.sin_family = AF_INET;
	addrFgSrv.sin_addr.S_un.S_addr = inet_addr(m_mapConfig.find(IPADDR)->second.c_str());
	addrFgSrv.sin_port = htons(atoi(m_mapConfig.find(PORT_FG2APP)->second.c_str()));
	//��׷���˵ĵ�ַ��Ϣ
	/*addrDgSrv.sin_family = AF_INET;
	addrDgSrv.sin_addr.S_un.S_addr = inet_addr(m_mapConfig.find(DG_IPADDR)->second.c_str());
	addrDgSrv.sin_port = htons(atoi(m_mapConfig.find(PORT_DG2APP)->second.c_str()));*/

	//2. ���ٻ����, ����ȫ�ֱ���
	memset(&gFgbuf, 0, sizeof(FGNetData));
	gFgbuf.version = FG_NET_FDM_VERSION;

	//3. �����̣߳���������fgfs�����ݣ��������ݴ���ڻ����
	HANDLE	hThread;
	DWORD	dwThread;
	bRecvThreadExit = FALSE;
	hThread = ::CreateThread(NULL, 0, RecvFromFgfs, (LPVOID)&gFgbuf, 0, &dwThread);
	if (hThread == NULL)
		return -1;

	CloseHandle(hThread);

	//4.���ض�̬��Dll_Test
	/*if ( !LoadDllTest() )
		return -1;*/
	//4. �˴�Ϊ���̣߳�������������дӲ���豸����дӲ���豸�ɹ���һ��socket�ͻ��˽���Ϣ���͸�fgfs��
	while (!nDebug && (!Open_Comm()))	//Debug=0˵��û�е���
	{
		cout << "Open_Comm Error.\r\n";
		Sleep(100);
	}

	do
	{//ִ��һ����ɡ����

		//0. ��ʼ��Ӳ��
		int reset = 0;

		cout << "����Ա�뿪����ʼӲ����λ..." << endl;
		cout << "��ɡѵ��ģ��ϵͳ��ʼ���У��Ƿ���и�λ������1���и�λ������0�����и�λ����";
		while (TRUE)
		{
			cin >> reset;
			if (reset == 1)
			{
				//��λ
				//1. �������4s
				InitDev();
				break;
			}

			if (reset == 0)
				break;

			cout << "���벻��ȷ�����������루����1���и�λ������0�����и�λ����";
		}

		//1. �ж��Ƿ�򿪲��ţ����ǣ�����Ʒ��ȴ���.
		if (!OpenFanByDoorStatus())
			break;

		//2. ��ʼ��������ȡ��̤���״̬Ȼ��Ӧ��fgfs�ϡ�
		if (!StartJump())
			break;
		
		//3. ��fgfs�յ�����������ʱ����ʼ�����������ɹ��󣬷��Ϳ��Ƶ��ƽ̨�����ݡ�
		//����ڴ˴������յ���׵��й����ݣ���ȥ���Ƶ�׵Ĳ��������ã�ͻȻ�½���ģ��������ʧ�ص���С�
		//OperateDg(10.0, 10.0, 10.0);	//�������
		/*if (!LoseWeight())	//Ӧ�÷���StartJump������ִ��
			break;*/

		//4. ��ɡ��ģ������(fgfs)�п���ɡ����Ϊ��ɡ�򿪺���һ�����ϵ�������
		//��˵����յ�fgfs�����Ĵ���ɡ�����ݵ�ʱ����Ҫ���Ƶ����ģ�����������
		//�����յ�������ɡû�д򿪵����ݣ���ô��Ҫ��������������ɡԱ��������ɡ��
		if (!OpenUmbrella())
			goto land;

		//5. Ϊ�˱�����ײ����ʱ��Ҫ������������
		/*if (!AvoidStriking())
			break;*/

		//6. ����������������������������ݿ��Ƶ��ƽ̨����б�˶���ͬʱ������fgfs.
		//������½��λ�ú��½��ٶ�
		if (!ControlLandingPos())
			break;
land:
		//7. ��½
		if (!Landing())
			break;
		
		cout << endl << endl;

		nRet = 0;
	} while (FALSE);

	bRecvThreadExit = TRUE;	//�˳������߳�
	Colse_Comm();
	WSACleanup();

	while (TRUE)
	{
		Sleep(200);
	}

	return nRet;
}

//if (fShift > 8)
//{//����
//	//turn right
//	fgbuf_tmp.oil_px[0] = (float)0.1*fShift;// 0.1 arc

//	//Send_Packeg_Udp_Data(FALL_DIANGANG, (FALL_DIANGANG - fShift > 0)?(FALL_DIANGANG - fShift):0, FALL_DIANGANG);	//�����������ģ����ק���, Ӳ������
//	bFlag = TRUE;
//}
//else if (fShift < -8)
//{//����
//	//turn left
//	fgbuf_tmp.oil_px[0] = (float)0.1*fShift;// 0.1 arc
//	cout << "psi send    " << fgbuf_tmp.oil_px[0] << endl;
//	//Send_Packeg_Udp_Data((FALL_DIANGANG + fShift > 0) ? (FALL_DIANGANG + fShift) : 0, FALL_DIANGANG, FALL_DIANGANG);
//	bFlag = TRUE;
//}
//else
//{//��ǰ�ƶ�
//	//Send_Packeg_Udp_Data(FALL_DIANGANG, FALL_DIANGANG, FALL_DIANGANG);
//	fgbuf_tmp.oil_px[0] = 120000;// 0.1 arc

//	/*if (iReadBlueData_L > 70000 && iReadBlueData_R > 70000)
//	{*/
//		//fgbuf_tmp.oil_px[1] = (float)0.016 * ((fproc_R <= fproc_L) ? fproc_R : fproc_L);
//	//}

//	fgbuf_tmp.oil_px[1] = 3000;

//	bFlag = TRUE;
//}