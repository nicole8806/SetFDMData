/**********************************************************
*Author: TY
*http://www.fg-china.net   
**********************************************************/

#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <iostream>
#include <cstdlib>

#include "FGNetData.h"
#include "GetConfig.h"
#include "Dll_Test/ComMoudle.h"
#include "Dll_Test/Center_Control.h"
#include "Dll_Test/DLL_Test.h"

using namespace std;

//��̤�忪��
#define FOOT_BOARD_SWITCH		(1<<5)		//��̤���ڵ�5λ����0��ʼ��������0���ɿ���1

//����ɡ����
#define PARACHUTE_SWITCH		(1<<7)		//����ɡ�����ڵ�7λ����0��ʼ,������0���ε���1

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
#define JYJ_RESET_LA_TIME		3300	//3300ms
#define JYJ_JUMP_FANG_TIME		2000	//-2000ms
#define JYJ_OPENCHUTE_LA_TIME	1000	//1000ms
#define JYJ_LAND_FANG_TIME		2300	//-2300ms
#endif

//��ײ���
//#define MIN_DG_HEIGHT			0.0
//#define MAX_DG_HEIGHT			380.0

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

//����3
#ifdef SLN_3

//#define INIT_DG_HEIGHT			200.0		//��ʼ����׸߶�
//#define LAND_DG_HEIGHT			100.0		//��½ʱ�ĵ�׸߶�
//#define OPEN_CHUTE_DG_HEIGHT		400.0		//��ɡ˲�佫��������ĸ߶�

#define INIT_DG_HEIGHT			100.0		//��ʼ����׸߶�
#define LAND_DG_HEIGHT			0.0		//��½ʱ�ĵ�׸߶�
#define OPEN_CHUTE_DG_HEIGHT	300.0		//��ɡ˲�佫��������ĸ߶�
#endif


//����
#define LAND_HEIGHT				164			//��½�߶�164Ӣ�磬����Լ5��
#define OPEN_CHUTE_TIME			4000		//��ȫ��ɡʱ�䣬4000ms����4s
//#define MAX_POS 330.0
//#define MIN_POS 300.0

map<string, string> m_mapConfig;

FGNetData			gFgbuf;
SOCKADDR_IN			addrFgSrv;		//Fg����˵�ַ��Ϣ

static int nDebug = 0;
static BOOL bRecvThreadExit = FALSE;
static BOOL	bOptDgTreadExit = FALSE;
static BOOL bOpenDoorFlag = FALSE;

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

	//1. ��׸�λ,�˴��Ƿ���ȷ��Ҫ������
	Creat_UDP();
	Send_Packeg_Udp_Data(INIT_DG_HEIGHT, INIT_DG_HEIGHT, INIT_DG_HEIGHT);

	//2. �رշ���
	Control_Fengshan(FS_CLOSE);

	//3. �������λ
	init_hardware();
	Sleep(50);
	Control_Juanyangji_info(JYJ_LA);
#if defined(SLN_1)
	SleepTime(JYJ_LAFANG_TIME);
#endif

#if defined(SLN_2)
	SleepTime(7000);
#endif

#if defined(SLN_3)
	SleepTime(JYJ_RESET_LA_TIME);
#endif
	
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

	bOpenDoorFlag = TRUE;	//�����Ѵ�

	return TRUE;
}

//��ʼ��������Fg����������Ϣ
BOOL StartJump()
{
	UCHAR		cPress_down = 0;
	FGNetData	fgbuf_tmp;
	int			nCount = 0;

	cout << "�����Ѵ򿪣�����..." << endl;

	//�ȼ���̤���Ƿ���£�����ȡ��̤���ֵ�Ƿ�Ϊ0
	do
	{
		if (!nDebug)
		{
			cPress_down = Get_Protect();	//��ȡ��̤��״̬��Ӳ������
		}
		else
		{
			cPress_down = ~FOOT_BOARD_SWITCH;
		}

		if ((cPress_down != 0) && !(cPress_down & FOOT_BOARD_SWITCH))
		{
			nCount++;
		}
		else
		{
			nCount = 0;
		}
		
		if (nCount == 3)
		{
			//cout << "��̤��պ�״̬" << endl;
			break;
		}
		//SleepTime(100);
	} while (TRUE);

	nCount = 0;

	//�ټ���̤���Ƿ��ɿ�������ȡ��̤���ֵ�Ƿ�Ϊ1
	do
	{
		if (nDebug)
			cPress_down = FOOT_BOARD_SWITCH;
		else
			cPress_down = Get_Protect();	//��ȡ��̤��״̬��Ӳ������

		if ((cPress_down != 0) && (cPress_down & FOOT_BOARD_SWITCH))
		{
			nCount++;
		}
		else
		{
			nCount = 0;
		}

		if (nCount == 3)
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

			if ((cPress_down != 0) && (cPress_down & PARACHUTE_SWITCH))
			{
				fgbuf_tmp = gFgbuf;
				fgbuf_tmp.fuel_px[3] = 10;
				fgbuf_tmp.ConverData();
				SOCKET	sockClient = socket(AF_INET, SOCK_DGRAM, 0);
				//sendto(sockClient, (char*)(&fgbuf_tmp), sizeof(fgbuf_tmp), 0, (SOCKADDR*)&addrFgSrv, sizeof(SOCKADDR));
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
		cout << "��ɡ�Ѵ�..." << endl;
	}
	
#if defined(SLN_1)
	SleepTime(4000);	//ʹ�۾��еĿ�ɡ��ʵ�ʵĿ�ɡ����һ��
#endif

#if defined(SLN_3)
	SleepTime(1000);	//��һ��ͣ
	if (!nDebug)
	{
		init_hardware();
		Sleep(50);
		Control_Juanyangji_info(JYJ_FANG);
		SleepTime(JYJ_JUMP_FANG_TIME);	//��2��͵�3�����ž����
		Control_Juanyangji_info(JYJ_STOP);
		Creat_UDP();
		Send_Packeg_Udp_Data(INIT_DG_HEIGHT - 100.0, INIT_DG_HEIGHT - 100.0, INIT_DG_HEIGHT - 100.0);	//��4���½�100����
		SleepTime(700);
	}
#endif

	if (!nDebug)
	{
		init_hardware();
		Sleep(50);
		Control_Juanyangji_info(JYJ_LA);
		Creat_UDP();
		Send_Packeg_Udp_Data(OPEN_CHUTE_DG_HEIGHT, OPEN_CHUTE_DG_HEIGHT, OPEN_CHUTE_DG_HEIGHT);	//�����������ģ����ק���, Ӳ������
		SleepTime(JYJ_OPENCHUTE_LA_TIME);
		Control_Juanyangji_info(JYJ_STOP);
		SleepTime(2000);
	}

	return TRUE;
}

DWORD WINAPI ShakeDg(LPVOID lpThreadParameter)
{
	DWORD	dwVal = 0;

	cout << "ShakeDg Thread Start." << endl;

	int	count = 0;
	long double t = 0;
	long double pi = 4.0 * atan(1.0);
	long double t1 = 10.0;
	long double t2 = 8.0;

	bool flag0 = false, flag1 = false;

	double base_pos = OPEN_CHUTE_DG_HEIGHT;
	int range = 150;

	int ii = 0;
	int n = 100 / 5;	

	Creat_UDP();
	//���������100mm
	for (ii = 0; ii < n; ii++)
	{
		base_pos -= 5;
		Send_Packeg_Udp_Data(base_pos, base_pos, base_pos);
		Sleep(100);
	}

	while (/*!bOptDgTreadExit*/count < 2)
	{
		if (!flag1)
		{
			Send_Packeg_Udp_Data(base_pos, base_pos + sin(t / 180.0 * pi) * range, base_pos - sin(t / 180.0 * pi) * range);
		}
		else
		{
			Send_Packeg_Udp_Data(base_pos, base_pos - sin(t / 180.0 * pi) * range, base_pos + sin(t / 180.0 * pi) * range);
		}

		if (!flag0)
		{
			t += t1;
			if (t >= 60.0)
			{
				t1 = 5.0;
			}

			if (t >= 90.0)
			{
				flag0 = true;
				//t = 90.0;
				t1 = 10.0;
			}

			Sleep(100);
		}
		else
		{
			if (t >= 90.0 && t < 270.0)
			{
				//t2 = 8.0;
				if (t >= 180.0)
					range = 120;
			}

			else if (t >= 270.0 && t < 450.0)
			{
				//t2 = 6.0;
				if (t >= 360.0)
					range = 90;
			}

			else if (t >= 450.0 && t < 630.0)
			{
				//t2 = 4.0;
				if (t >= 540.0)
					range = 60;

			}

			else if (t >= 630.0 && t <= 720.0)
			{
				//t2 = 2.0;
				/*if (t >= 540.0)
				fudu = 30;*/
			}

			else if (t > 720.0)
			{
				t = 0.0;
				count++;
			}

			t += t2;
			Sleep(100);
		}
	}

	/*Sleep(200);
	Send_Packeg_Udp_Data(OPEN_CHUTE_DG_HEIGHT, OPEN_CHUTE_DG_HEIGHT, OPEN_CHUTE_DG_HEIGHT);*/

	return dwVal;
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

	if (!nDebug)
	{
		hThread = ::CreateThread(NULL, 0, ShakeDg, NULL, 0, &dwThreadId);
		if (hThread == NULL)
			return -1;

		CloseHandle(hThread);

		nLeftFrontInit = Get_Pull_Data(LEFT_FRONT_DRAG);
		nLeftAfterInit = Get_Pull_Data(LEFT_AFTER_DRAG);
		nRightFrontInit = Get_Pull_Data(RIGHT_FRONT_DRAG);
		nRightAfterInit = Get_Pull_Data(RIGHT_AFTER_DRAG);
	}

	do{
		SleepTime(300);

		if (nDebug)
		{
			nRightAfterDrag = 16000;
			nLeftAfterDrag = 0;
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

		/*5�׵�ʱ����ʾ������½*/
		if (!bLandFlag && (fgbuf_tmp.mp_osi[1] <= (fgbuf_tmp.fuel_px[3] + LAND_HEIGHT)))
		{
			cout << "������½����׼��..." << endl;
			bOptDgTreadExit = TRUE;
			bLandFlag = TRUE;
		}

		//�����߶Ȳ���5m������Ҫ��ɡ��ֱ����½
		//�˴����뵥λ��Ӣ�磬�ò����������Ǻ��Σ������ɽ�غ�����fgbuf_tmp.fuel_px[3]Ӣ�磬2.5m��Ϊ82Ӣ�硣
		if (fgbuf_tmp.mp_osi[1] <= (fgbuf_tmp.fuel_px[3] + /*LAND_HEIGHT*/82))
		{
			//��ʼ��½��ֹͣ��������
			fgbuf_tmp.oil_px[0] = (float)0.0;
			fgbuf_tmp.oil_temp[3] = (float)0.0;
			fgbuf_tmp.ConverData();
			sendto(sockClient, (char*)(&fgbuf_tmp), sizeof(fgbuf_tmp), 0, (SOCKADDR*)&addrFgSrv, sizeof(SOCKADDR));

			Sleep(2500);
			break;
		}

		cout << "fgbuf_tmp.mp_osi[1]: " << fgbuf_tmp.mp_osi[1] << endl;
		cout << "fgbuf_tmp.fuel_px[3]: " << fgbuf_tmp.fuel_px[3] << endl;

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

		/*cout << "fgbuf_tmp.oil_px[0]: " << fgbuf_tmp.oil_px[0] << endl;
		cout << "fgbuf_tmp.oil_px[1]: " << fgbuf_tmp.oil_px[1] << endl;*/

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
		init_hardware();
		Sleep(50);
		Control_Juanyangji_info(JYJ_FANG);
#if defined(SLN_1)
		SleepTime(JYJ_LAFANG_TIME);
#elif defined(SLN_2)
		SleepTime(7000);
#elif defined(SLN_3)
		Send_Packeg_Udp_Data(LAND_DG_HEIGHT, LAND_DG_HEIGHT, LAND_DG_HEIGHT);
		SleepTime(JYJ_LAND_FANG_TIME);
#endif
		Control_Juanyangji_info(JYJ_STOP);
	}
	
	//cout << "Landing is over..." << endl;
	cout << "��½�ɹ�������Աж��ɡ�������볡..." << endl;

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
	//addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrSrv.sin_addr.S_un.S_addr = inet_addr(m_mapConfig.find(IPADDR_PILOT)->second.c_str());
	addrSrv.sin_port = htons(atoi(m_mapConfig.find(PORT_APP2FG)->second.c_str()));	//�˿�5300
	bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));

	cout << m_mapConfig.find(IPADDR_PILOT)->second.c_str() << endl;
	//2. ��������
	SOCKADDR_IN addrClient;
	FGNetData	fgbuf_tmp;
	int			len = sizeof(SOCKADDR);
	int			err = 0;

	//cout << "RecvFromFgfs" << endl;

	while (!bRecvThreadExit)
	{
		memset(&fgbuf_tmp, 0, sizeof(fgbuf_tmp));

		err = recvfrom(sockSrv, (char *)(&fgbuf_tmp), sizeof(fgbuf_tmp), 0, (SOCKADDR*)&addrClient, &len);
		
		if (err != -1)
		{
			fgbuf_tmp.RecvConverData();
			memcpy(pFgfuf, &fgbuf_tmp, sizeof(FGNetData));
		}

		if (bOpenDoorFlag)
		{
			closesocket(sockSrv);
			sockSrv = socket(AF_INET, SOCK_DGRAM, 0);	//UDP
			addrSrv.sin_addr.S_un.S_addr = inet_addr(m_mapConfig.find(IPADDR)->second.c_str());
			bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
			bOpenDoorFlag = FALSE;
		}

		//cout << "��������" << endl;
		Sleep(100);
	}

	closesocket(sockSrv);
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
	//cout << m_mapConfig.find(IPADDR)->second.c_str() << endl;
	//cout << m_mapConfig.find(PORT_FG2APP)->second.c_str() << endl;
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

		//4. ��ɡ��ģ�����(fgfs)�п���ɡ����Ϊ��ɡ�򿪺���һ�����ϵ�������
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

	system("pause");
	//Sleep(8000);

	return nRet;
}
