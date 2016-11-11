#include "DevControl.h"

extern HANDLE hCom1, hCom2;

//OVERLAPPED m_osRead;
//OVERLAPPED m_osWrite;
//COMSTAT ComStat;
//DWORD dwErrorFlags;
//DWORD dwBytesRead = 1024;
//char ReadBuf[100];

SOCKADDR_IN addrDgSrv;		//电缸服务端地址信息

/*******************************************************
1，函数的调用的前提是调用Open_Comm()的返回值为“1”，串口打开成功
2, 函数返回表示读取到的编码器数据，取值为负表示反方向运动位置，为正表示正方向运动位置
3，参数Encoder_chanel取值范围为‘1’和‘2’，表示打开两个不同编码器通道
********************************************************/
unsigned int Read_Encoder(int Encoder_chanel)
{
	//读编码器数据
	UCHAR Encoder_Buf[5] = {0};
	Encoder_Buf[0] = 0xff;
	if (Encoder_chanel == 1)
		Encoder_Buf[2] = 0x16;
	else
		Encoder_Buf[2] = 0x17;

	//发送读编码器指令
	int write_encode = -1;
	write_encode = Write_Comm(hCom1, Encoder_Buf, 5);
	Sleep(100);

	//接收单板编码器数据
	int read_encode = -1;
	memset(Encoder_Buf, 0, 5);
	read_encode = Read_Comm(hCom1, Encoder_Buf, 5);

	UINT Encoder_data = 0;
	Encoder_data = ((UINT)Encoder_Buf[4] << 8) | Encoder_Buf[3];
	
	Encoder_data = 66000 + Encoder_data;
	if (Encoder_data < 65000)
	{
		Encoder_data = 65536 + Encoder_data;
	}

	return Encoder_data;
}


/*******************************************************
1，函数的调用的前提是调用Open_Comm()的返回值为“1”，串口打开成功
2, 函数返回表示读取到备份伞和脚踏开关的状态，是一个字节数据，可以转化为8位二进制，
其中脚踏开关在返回值的第8位位，1表示没有踩下，0表示踩下了。备份伞在第7位，1表示拉出了备份伞，0表示没有拉出。
3，参数Encoder_chanel取值范围为‘1’和‘2’，表示打开两个不同编码器通道
********************************************************/
unsigned char Read_Protect()
{
	//读压力计数据
	UCHAR Protect_Buf[] = { 0xff, 0x20, 0x20, 0x00, 0x00 };

	DWORD	dwSize = sizeof(Protect_Buf);
	//发送读读压力计数据指令
	int write_encode = -1;
	write_encode = Write_Comm(hCom2, Protect_Buf, dwSize);
	Sleep(100);
	//接收单板读压力计数据
	int read_Protect = -1;
	memset(Protect_Buf, 0, dwSize);
	read_Protect = Read_Comm(hCom2, Protect_Buf, dwSize);

	return Protect_Buf[3];
}

/*******************************************************
拉力传感器
********************************************************/
unsigned int Read_Presure(int channel)
{
	//读编码器数据
	UCHAR Presure_Buf[5] = {0};
	Presure_Buf[0] = 0xff;
	Presure_Buf[1] = 0x20;
	if (channel == 1)
		Presure_Buf[2] = 0x18;
	else
		Presure_Buf[2] = 0x19;

	//发送读读压力计数据指令
	int write_encode = -1;
	write_encode = Write_Comm(hCom2, Presure_Buf, 5);
	Sleep(10);
	//接收单板读压力计数据
	int read_Protect = -1;
	memset(Presure_Buf, 0, 5);
	read_Protect = Read_Comm(hCom2, Presure_Buf, 5);
	return (Presure_Buf[3] * 256 + Presure_Buf[4]);
}


/*******************************************************
1，函数的调用的前提是调用Open_Comm()的返回值为“1”，串口打开成功
2, 函数返回1表示控制失败，0表示控制成功
3，参数flag 1表示打开风扇，0表示关闭风扇
********************************************************/
int control_Fan(int flag)
{
	//读编码器数据
	UCHAR Fan_Buf[5] = {0};
	Fan_Buf[0] = 0xff;
	Fan_Buf[1] = 0x23;
	Fan_Buf[2] = 0x00;
	if (flag == 1)
		Fan_Buf[3] = 0x64;
	else
		Fan_Buf[3] = 0x00;

	Fan_Buf[4] = 0x00;

	//发送读读压力计数据指令
	int write_encode = -1;
	write_encode = Write_Comm(hCom2, Fan_Buf, 5);
	if (write_encode < 5)
	{
		return 1;
	}

	return 0;
}

/*******************************************************
1，控制电缸平台运动函数
2, 函数返回1表示控制失败，0表示控制成功
3，参数表示三个电缸运动的目的位置
********************************************************/
int control_Diangang(double x, double y, double z)
{
	//暂时不做
	return 0;
}

/***************************************************************************
**********把位置数据作为参数，打包处理成符合运动平台UDP协议的网络包**********
****************************************************************************/
int Send_Packeg_Udp_Data(double x_Po, double y_Po, double z_Po)
{
	//电缸参数赋值
	const double	AccesssDistanceUnit = 400.0;
	const double	LeadDistanceUnit	= 5.0;
	const long		oneTurnPluseNum		= 10000;
	const int		AcceptIPNode		= 255;
	const int		AcceptIPGroup		= 255;
	const int		ReplyIPGroup		= 255;
	const int		ReplyIPNode			= 255;

	//定义每轴脉冲数
	long XPulse; //脉冲个数对应,1号轴
	long YPulse; //脉冲个数对应，2号轴
	long ZPulse; //脉冲个数对应3号轴
	//long UPulse; //脉冲个数对应4号轴
	//long VPulse; //脉冲个数对应5号轴
	//long WPulse; //脉冲个数对应6号轴

	XPulse = x_Po / LeadDistanceUnit * oneTurnPluseNum;
	YPulse = y_Po / LeadDistanceUnit * oneTurnPluseNum;
	ZPulse = z_Po / LeadDistanceUnit * oneTurnPluseNum;
	//UPulse = AccesssDistanceUnit / LeadDistanceUnit * oneTurnPluseNum;
	//VPulse = AccesssDistanceUnit / LeadDistanceUnit * oneTurnPluseNum;
	//WPulse = AccesssDistanceUnit / LeadDistanceUnit * oneTurnPluseNum;

	//UDP数据区赋值
	const int ConfirmCode = 0x55aa; //确认码
	const int PassCode = 0x0000; //通过码
	const int FunctionCode = 0x1301; //功能码：1301绝对时间播放功能码，1401相对时间播放功能码
	const int ChannelCode = 0x0000;  //0000：平台是三轴；0001：平台是六轴

	const int WhoAcceptCode = 0xffff; //所有平台都接收
	const int WhoReplyCode = 0xffff; //所有平台都应答

	const long PlayLine = 0x00; //播放UDP指令序列，绝对时间播放可设置为0
	const long PlayTime = 0x00; //播放时间设置为零，按MBOX控制板卡内部时间播放
	const long PlayXpos = XPulse; //1号缸位置脉冲数
	const long PlayYpos = YPulse; //2好缸位置脉冲数
	const long PlayZpos = ZPulse; //3号缸位置脉冲数

	const int  BaseDoutCode = 0x1234; //12路数字开关量输出,低12位有效

	const int  DacOneCode = 0x5678; //模拟量1的输出
	const int  DacTwoCode = 0xabcd; //模拟量2的输出

	//定义存放UDO数据的数组
	char Buffer[49];
	char UDPBuffer[49];

	//将双字和字分成字节，并高位在前低位在后
	memcpy(Buffer, &ConfirmCode, 2);
	UDPBuffer[0] = Buffer[1];
	UDPBuffer[1] = Buffer[0];

	memcpy(&Buffer[2], &PassCode, 2);
	UDPBuffer[2] = Buffer[3];
	UDPBuffer[3] = Buffer[2];

	memcpy(&Buffer[4], &FunctionCode, 2);
	UDPBuffer[4] = Buffer[5];
	UDPBuffer[5] = Buffer[4];

	memcpy(&Buffer[6], &ChannelCode, 2);
	UDPBuffer[6] = Buffer[7];
	UDPBuffer[7] = Buffer[6];

	memcpy(&Buffer[8], &WhoAcceptCode, 2);
	UDPBuffer[8] = Buffer[9];
	UDPBuffer[9] = Buffer[8];

	memcpy(&Buffer[10], &WhoReplyCode, 2);
	UDPBuffer[10] = Buffer[10];
	UDPBuffer[11] = Buffer[11];

	memcpy(&Buffer[12], &PlayLine, 4);
	UDPBuffer[12] = Buffer[15];
	UDPBuffer[13] = Buffer[14];
	UDPBuffer[14] = Buffer[13];
	UDPBuffer[15] = Buffer[12];

	memcpy(&Buffer[16], &PlayTime, 4);
	UDPBuffer[16] = Buffer[19];
	UDPBuffer[17] = Buffer[18];
	UDPBuffer[18] = Buffer[17];
	UDPBuffer[19] = Buffer[16];

	memcpy(&Buffer[20], &PlayXpos, 4);
	UDPBuffer[20] = Buffer[23];
	UDPBuffer[21] = Buffer[22];
	UDPBuffer[22] = Buffer[21];
	UDPBuffer[23] = Buffer[20];

	memcpy(&Buffer[24], &PlayYpos, 4);
	UDPBuffer[24] = Buffer[27];
	UDPBuffer[25] = Buffer[26];
	UDPBuffer[26] = Buffer[25];
	UDPBuffer[27] = Buffer[24];

	memcpy(&Buffer[28], &PlayZpos, 4);
	UDPBuffer[28] = Buffer[31];
	UDPBuffer[29] = Buffer[30];
	UDPBuffer[30] = Buffer[29];
	UDPBuffer[31] = Buffer[28];

	memcpy(&Buffer[32], &BaseDoutCode, 2);
	UDPBuffer[32] = Buffer[33];
	UDPBuffer[33] = Buffer[32];

	memcpy(&Buffer[34], &DacOneCode, 2);
	UDPBuffer[34] = Buffer[34];
	UDPBuffer[35] = Buffer[35];

	memcpy(&Buffer[34], &DacTwoCode, 2);
	UDPBuffer[36] = Buffer[37];
	UDPBuffer[37] = Buffer[36];

	SOCKET sockClient;
	if ((sockClient = socket(AF_INET, SOCK_DGRAM, 0)) <= 0)
		return -1;

	sendto(sockClient, UDPBuffer, 38, 0, (struct sockaddr*)&addrDgSrv, sizeof(addrDgSrv));

	closesocket(sockClient);

	Sleep(100);
	return 1;
}

/*初始化*/
BOOL Init()
{
	//初始化卷扬机
	UCHAR byCmd1[] = { 0xff, 0x23, 0x00, 0x00, 0x00 };
	UCHAR byCmd2[] = { 0xff, 0x23, 0x01, 0x00, 0x00 };
	UCHAR byCmd3[] = { 0xff, 0x23, 0x02, 0x00, 0x00 };
	UCHAR byCmd4[] = { 0xff, 0x23, 0x03, 0x00, 0x00 };

	Write_Comm(hCom2, byCmd1, 5);
	Sleep(100);
	Write_Comm(hCom2, byCmd2, 5);
	Sleep(100);
	Write_Comm(hCom2, byCmd3, 5);
	Sleep(100);
	Write_Comm(hCom2, byCmd4, 5);
	
	return TRUE;
}

#define JUANJI_SLEEP_TIME	50

/*开始拉卷扬机*/
BOOL StartLaJYJ()
{
	BOOL bRet = FALSE;
	
	//顺序不能掉
	UCHAR byCmd1[] = { 0xff, 0x23, 0x03, 0x64, 0x00 };
	UCHAR byCmd2[] = { 0xff, 0x23, 0x01, 0x64, 0x00 };

	Write_Comm(hCom2, byCmd1, 5);
	Sleep(JUANJI_SLEEP_TIME);
	Write_Comm(hCom2, byCmd2, 5);

	return bRet;
}

//停止拉卷扬机
BOOL StopLaJYJ()
{
	BOOL bRet = FALSE;

	UCHAR byCmd3[] = { 0xff, 0x23, 0x01, 0x00, 0x00 };
	UCHAR byCmd4[] = { 0xff, 0x23, 0x03, 0x00, 0x00 };

	Write_Comm(hCom2, byCmd3, 5);
	Sleep(JUANJI_SLEEP_TIME);
	Write_Comm(hCom2, byCmd4, 5);

	return bRet;
}

/*开始放卷扬机*/
BOOL StartFangJYJ()
{
	BOOL bRet = FALSE;

	//顺序不能掉
	UCHAR byCmd1[] = { 0xff, 0x23, 0x02, 0x64, 0x00 };
	UCHAR byCmd2[] = { 0xff, 0x23, 0x01, 0x64, 0x00 };

	Write_Comm(hCom2, byCmd1, 5);
	Sleep(JUANJI_SLEEP_TIME);
	Write_Comm(hCom2, byCmd2, 5);

	return bRet;
}

//停止放卷扬机
BOOL StopFangJYJ()
{
	BOOL bRet = FALSE;

	UCHAR byCmd3[] = { 0xff, 0x23, 0x01, 0x00, 0x00 };
	UCHAR byCmd4[] = { 0xff, 0x23, 0x02, 0x00, 0x00 };

	Write_Comm(hCom2, byCmd3, 5);
	Sleep(JUANJI_SLEEP_TIME);
	Write_Comm(hCom2, byCmd4, 5);

	return bRet;
}