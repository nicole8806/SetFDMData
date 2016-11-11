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

//脚踏板开关
#define FOOT_BOARD_SWITCH		(1<<5)		//脚踏板在第5位，从0开始，踩下是0，松开是1

//备份伞开关
#define PARACHUTE_SWITCH		(1<<7)		//备份伞开关在第7位，从0开始,插上是0，拔掉是1

//风扇开关
#define FS_OPEN		1
#define FS_CLOSE	0

//拉绳(操纵棒)方向
#define RIGHT_FRONT_DRAG		1	//右前拉绳
#define RIGHT_AFTER_DRAG		2	//右后拉绳
#define LEFT_AFTER_DRAG			3	//左后拉绳
#define LEFT_FRONT_DRAG			4	//左前拉绳

//操纵带

//#define SLN_1
//#define SLN_2
#define SLN_3

//卷扬机拉放
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

//电缸参数
//#define MIN_DG_HEIGHT			0.0
//#define MAX_DG_HEIGHT			380.0

//方案1
#ifdef SLN_1
#define INIT_DG_HEIGHT			200.0		//初始化电缸高度
#define LAND_DG_HEIGHT			100.0		//着陆时的电缸高度
#define OPEN_CHUTE_DG_HEIGHT	350.0		//开伞瞬间将电缸拉到的高度
#endif

//方案2
#ifdef SLN_2
#define INIT_DG_HEIGHT			0.0		//初始化电缸高度
#define JYJ_LA_TIME				
#define JYJ_FANG_TIME				
#define OPEN_CHUTE_DG_HEIGHT	400.0		//开伞瞬间将电缸拉到的高度
#endif

//方案3
#ifdef SLN_3

//#define INIT_DG_HEIGHT			200.0		//初始化电缸高度
//#define LAND_DG_HEIGHT			100.0		//着陆时的电缸高度
//#define OPEN_CHUTE_DG_HEIGHT		400.0		//开伞瞬间将电缸拉到的高度

#define INIT_DG_HEIGHT			100.0		//初始化电缸高度
#define LAND_DG_HEIGHT			0.0		//着陆时的电缸高度
#define OPEN_CHUTE_DG_HEIGHT	300.0		//开伞瞬间将电缸拉到的高度
#endif


//其它
#define LAND_HEIGHT				164			//着陆高度164英寸，即大约5米
#define OPEN_CHUTE_TIME			4000		//完全开伞时间，4000ms，即4s
//#define MAX_POS 330.0
//#define MIN_POS 300.0

map<string, string> m_mapConfig;

FGNetData			gFgbuf;
SOCKADDR_IN			addrFgSrv;		//Fg服务端地址信息

static int nDebug = 0;
static BOOL bRecvThreadExit = FALSE;
static BOOL	bOptDgTreadExit = FALSE;
static BOOL bOpenDoorFlag = FALSE;

void SleepTime(_In_ DWORD dwMilliseconds)
{
	DWORD		dwTime = GetTickCount();
	while ((GetTickCount() - dwTime) < dwMilliseconds)
	{
		Sleep(20);	//保证误差不会超过20ms
	}	
}

//初始化设备
BOOL InitDev()
{
	//cout << "Initializing Device......" << endl;
	cout << "正在初始化硬件..." << endl;

	if (nDebug)
		goto end;

	//1. 电缸复位,此处是否正确需要再讨论
	Creat_UDP();
	Send_Packeg_Udp_Data(INIT_DG_HEIGHT, INIT_DG_HEIGHT, INIT_DG_HEIGHT);

	//2. 关闭风扇
	Control_Fengshan(FS_CLOSE);

	//3. 卷扬机复位
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
	cout << "硬件复位完成，请跳伞员穿戴伞具，戴上3D眼镜，踩好脚踏板，等待命令准备跳伞..." << endl;

	return TRUE;
}

//根据舱门状态控制风扇运动
BOOL OpenFanByDoorStatus()
{
	FGNetData	fgbuf_tmp = gFgbuf;

	//cout << "Waiting to open door......" << endl;
	cout << "开舱门..." << endl;
	while (fgbuf_tmp.mp_osi[2] != 10)	//舱门没打开，一直等待
	{
		fgbuf_tmp = gFgbuf;
		Sleep(50);
	}

	//cout << "Door is opend, now open the fan......" << endl;
	if (!nDebug)
		Control_Fengshan(FS_OPEN);	//控制风扇吹，//硬件操作

	bOpenDoorFlag = TRUE;	//舱门已打开

	return TRUE;
}

//开始起跳，向Fg发送起跳信息
BOOL StartJump()
{
	UCHAR		cPress_down = 0;
	FGNetData	fgbuf_tmp;
	int			nCount = 0;

	cout << "舱门已打开，起跳..." << endl;

	//先检测脚踏板是否踩下，即读取脚踏板的值是否为0
	do
	{
		if (!nDebug)
		{
			cPress_down = Get_Protect();	//读取脚踏板状态，硬件操作
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
			//cout << "脚踏板闭合状态" << endl;
			break;
		}
		//SleepTime(100);
	} while (TRUE);

	nCount = 0;

	//再检测脚踏板是否松开，即读取脚踏板的值是否为1
	do
	{
		if (nDebug)
			cPress_down = FOOT_BOARD_SWITCH;
		else
			cPress_down = Get_Protect();	//读取脚踏板状态，硬件操作

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

			//Send_Packeg_Udp_Data(100.0, 100.0, 100.0);	//跳伞之后会有失重的感觉

			//cout << "Jump success......" << endl;
			break;
		}
	} while (TRUE);

	return TRUE;
}

//打开主伞
BOOL OpenUmbrella()
{
	FGNetData	fgbuf_tmp = gFgbuf;
	UCHAR		cPress_down = 0;
	DWORD		dwJumpTime;
	DWORD		dwTimeInternal = 0;
	BOOL		bOpenFail = FALSE;

	//cout << "Waiting to open main umbrella......" << endl;

	dwJumpTime = GetTickCount();

	cout << "正在打开主伞..." << endl;
	while (fgbuf_tmp.mp_osi[3] != 10)	//主伞没打开，一直等待
	{
		fgbuf_tmp = gFgbuf;
		Sleep(50);

		dwTimeInternal = GetTickCount() - dwJumpTime;
		//4s时间，主伞没有打开：跳伞后开始开伞，开伞的时间为4s,若4s内没有打开主伞，则认为主伞打开失败，应该去打开备份伞
		if (dwTimeInternal >= OPEN_CHUTE_TIME)
		{
			bOpenFail = TRUE;
			break;
		}
	}

	if (bOpenFail)
	{
		//cout << "Open main umbrella fail, waiting to open backup umbrella......" << endl;
		cout << "主伞没有打开，请打开备份伞..." << endl;
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
				cout << "备份伞已打开..." << endl;
				break;
			}

			if (fgbuf_tmp.mp_osi[1] <= (fgbuf_tmp.fuel_px[3] + LAND_HEIGHT))	//离地面高度不足5m，则不需要控伞，直接着陆
			{
				cout << "备份伞没有打开，此次跳伞模拟失败，模拟人员可能已经伤亡..." << endl;
				cout << "开始着陆..." << endl;
				return FALSE;
			}

		} while (TRUE);
	}
	else
	{
		cout << "主伞已打开..." << endl;
	}
	
#if defined(SLN_1)
	SleepTime(4000);	//使眼镜中的开伞与实际的开伞拉起一致
#endif

#if defined(SLN_3)
	SleepTime(1000);	//第一秒停
	if (!nDebug)
	{
		init_hardware();
		Sleep(50);
		Control_Juanyangji_info(JYJ_FANG);
		SleepTime(JYJ_JUMP_FANG_TIME);	//第2秒和第3秒拉放卷扬机
		Control_Juanyangji_info(JYJ_STOP);
		Creat_UDP();
		Send_Packeg_Udp_Data(INIT_DG_HEIGHT - 100.0, INIT_DG_HEIGHT - 100.0, INIT_DG_HEIGHT - 100.0);	//第4秒下降100毫米
		SleepTime(700);
	}
#endif

	if (!nDebug)
	{
		init_hardware();
		Sleep(50);
		Control_Juanyangji_info(JYJ_LA);
		Creat_UDP();
		Send_Packeg_Udp_Data(OPEN_CHUTE_DG_HEIGHT, OPEN_CHUTE_DG_HEIGHT, OPEN_CHUTE_DG_HEIGHT);	//电缸向上拉起，模拟拉拽体感, 硬件操作
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
	//电缸慢慢放100mm
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

//控制着陆的位置和下降的速度,拉绳。
BOOL ControlLandingPos()
{
	//判断离地面的距离
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
	cout << "正在下降，请拉操纵袋或操纵棒来控制着陆的位置和下降的速度..." << endl;

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

		//修正为0
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

		/*5米的时候提示即将着陆*/
		if (!bLandFlag && (fgbuf_tmp.mp_osi[1] <= (fgbuf_tmp.fuel_px[3] + LAND_HEIGHT)))
		{
			cout << "即将着陆，请准备..." << endl;
			bOptDgTreadExit = TRUE;
			bLandFlag = TRUE;
		}

		//离地面高度不足5m，则不需要控伞，直接着陆
		//此处距离单位是英寸，该参数描述的是海拔，降落的山地海拔是fgbuf_tmp.fuel_px[3]英寸，2.5m即为82英寸。
		if (fgbuf_tmp.mp_osi[1] <= (fgbuf_tmp.fuel_px[3] + /*LAND_HEIGHT*/82))
		{
			//开始着陆，停止拉绳控制
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
		//左右转动
		fgbuf_tmp.oil_px[0] = nShift;	//左右拉力差
		//水平速度
		fgbuf_tmp.oil_px[1] = 6000 - min(min(nRightAfterDrag, nLeftAfterDrag) / 16820.0, 1.0) * 6000 \
								+ min(min(nRightFrontDrag, nLeftFrontDrag) / 16820.0, 1.0) * 6000;	//单位：mm/s	
		
		//上下抖动
		//fgbuf_tmp.oil_px[2] = 3;
		//左右抖动
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

//着陆
BOOL Landing()
{
	//cout << "Falling is over, Now is Landing..." << endl;
	//1. 关闭风扇
	if (!nDebug)
	{
		Control_Fengshan(FS_CLOSE);

#if defined(SLN_1)
		Send_Packeg_Udp_Data(LAND_DG_HEIGHT, LAND_DG_HEIGHT, LAND_DG_HEIGHT);
#endif

		//3. 控制卷扬机，进行着陆，跳伞流程完成。
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
	cout << "着陆成功，请人员卸掉伞具自行离场..." << endl;

	return TRUE;
}

//从fgfs接收数据线程入口函数
DWORD WINAPI RecvFromFgfs(LPVOID lpParam)
{
	FGNetData *pFgfuf = (FGNetData*)lpParam;

	//1. 创建套接字并绑定IP地址和端口号
	SOCKET sockSrv = socket(AF_INET, SOCK_DGRAM, 0);	//UDP
	SOCKADDR_IN addrSrv;
	addrSrv.sin_family = AF_INET;
	//addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrSrv.sin_addr.S_un.S_addr = inet_addr(m_mapConfig.find(IPADDR_PILOT)->second.c_str());
	addrSrv.sin_port = htons(atoi(m_mapConfig.find(PORT_APP2FG)->second.c_str()));	//端口5300
	bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));

	cout << m_mapConfig.find(IPADDR_PILOT)->second.c_str() << endl;
	//2. 接收数据
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

		//cout << "正在运行" << endl;
		Sleep(100);
	}

	closesocket(sockSrv);
	//cout << "停止运行" << endl;
	return 0;
}

int main(int argc, char *argv[])
{
	int	nRet = -1;
	TCHAR szModulePath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szModulePath, MAX_PATH);
	(_tcsrchr(szModulePath, _T('\\')))[1] = '\0';
	strcat_s(szModulePath, CONFIG_FILE);

	//1. 读取配置文件
	string s_filePath = szModulePath;
	ReadConfig(s_filePath, m_mapConfig);
	nDebug = atoi(m_mapConfig.find(DEBUG_NO_HARDWARE)->second.c_str());
	//cout << nDebug << endl;

	//2. 本进程要用到socket通讯，所以先初始化socket版本
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
		return -1;
	}
	//Fg服务端的地址信息
	addrFgSrv.sin_family = AF_INET;
	addrFgSrv.sin_addr.S_un.S_addr = inet_addr(m_mapConfig.find(IPADDR)->second.c_str());
	addrFgSrv.sin_port = htons(atoi(m_mapConfig.find(PORT_FG2APP)->second.c_str()));
	//cout << m_mapConfig.find(IPADDR)->second.c_str() << endl;
	//cout << m_mapConfig.find(PORT_FG2APP)->second.c_str() << endl;
	//电缸服务端的地址信息
	/*addrDgSrv.sin_family = AF_INET;
	addrDgSrv.sin_addr.S_un.S_addr = inet_addr(m_mapConfig.find(DG_IPADDR)->second.c_str());
	addrDgSrv.sin_port = htons(atoi(m_mapConfig.find(PORT_DG2APP)->second.c_str()));*/

	//2. 开辟缓存块, 定义全局变量
	memset(&gFgbuf, 0, sizeof(FGNetData));
	gFgbuf.version = FG_NET_FDM_VERSION;

	//3. 创建线程，用来接收fgfs的数据，并将数据存放在缓存块
	HANDLE	hThread;
	DWORD	dwThread;
	bRecvThreadExit = FALSE;
	hThread = ::CreateThread(NULL, 0, RecvFromFgfs, (LPVOID)&gFgbuf, 0, &dwThread);
	if (hThread == NULL)
		return -1;

	CloseHandle(hThread);

	//4. 此处为主线程，根据命令来读写硬件设备，读写硬件设备成功后开一个socket客户端将信息发送给fgfs。
	while (!nDebug && (!Open_Comm()))	//Debug=0说明没有调试
	{
		cout << "Open_Comm Error.\r\n";
		Sleep(100);
	}

	do
	{//执行一次跳伞流程

		//0. 初始化硬件
		int reset = 0;

		cout << "请人员离开，开始硬件复位..." << endl;
		cout << "跳伞训练模拟系统开始运行，是否进行复位（输入1进行复位，输入0不进行复位）：";
		while (TRUE)
		{
			cin >> reset;
			if (reset == 1)
			{
				//复位
				//1. 拉卷扬机4s
				InitDev();
				break;
			}

			if (reset == 0)
				break;

			cout << "输入不正确，请重新输入（输入1进行复位，输入0不进行复位）：";
		}

		//1. 判断是否打开舱门，若是，则控制风扇吹风.
		if (!OpenFanByDoorStatus())
			break;

		//2. 开始起跳，读取脚踏板的状态然后反应到fgfs上。
		if (!StartJump())
			break;
		
		//3. 当fgfs收到起跳的命令时，开始起跳，起跳成功后，发送控制电缸平台的数据。
		//因此在此处：若收到电缸的有关数据，便去控制电缸的操作，作用：突然下降，模拟起跳后失重的体感。
		//OperateDg(10.0, 10.0, 10.0);	//操作电缸
		/*if (!LoseWeight())	//应该放在StartJump函数中执行
			break;*/

		//4. 开伞，模拟软件(fgfs)中开主伞，因为主伞打开后有一个向上的拉力，
		//因此当接收到fgfs发来的打开主伞的数据的时候需要控制电缸来模拟这个拉力。
		//若接收到的是主伞没有打开的数据，那么就要控制音响提醒跳伞员开启备份伞。
		if (!OpenUmbrella())
			goto land;

		//5. 为了避免相撞，此时需要控制拉力传感
		/*if (!AvoidStriking())
			break;*/

		//6. 拉绳，根据拉绳箱的左右拉绳数据控制电缸平台的倾斜运动，同时反馈给fgfs.
		//控制着陆的位置和下降速度
		if (!ControlLandingPos())
			break;
land:
		//7. 着陆
		if (!Landing())
			break;
		
		cout << endl << endl;

		nRet = 0;
	} while (FALSE);

	bRecvThreadExit = TRUE;	//退出接收线程
	Colse_Comm();
	WSACleanup();

	system("pause");
	//Sleep(8000);

	return nRet;
}
