#include "DevControl.h"

extern HANDLE hCom1, hCom2;

//OVERLAPPED m_osRead;
//OVERLAPPED m_osWrite;
//COMSTAT ComStat;
//DWORD dwErrorFlags;
//DWORD dwBytesRead = 1024;
//char ReadBuf[100];

SOCKADDR_IN addrDgSrv;		//��׷���˵�ַ��Ϣ

/*******************************************************
1�������ĵ��õ�ǰ���ǵ���Open_Comm()�ķ���ֵΪ��1�������ڴ򿪳ɹ�
2, �������ر�ʾ��ȡ���ı��������ݣ�ȡֵΪ����ʾ�������˶�λ�ã�Ϊ����ʾ�������˶�λ��
3������Encoder_chanelȡֵ��ΧΪ��1���͡�2������ʾ��������ͬ������ͨ��
********************************************************/
unsigned int Read_Encoder(int Encoder_chanel)
{
	//������������
	UCHAR Encoder_Buf[5] = {0};
	Encoder_Buf[0] = 0xff;
	if (Encoder_chanel == 1)
		Encoder_Buf[2] = 0x16;
	else
		Encoder_Buf[2] = 0x17;

	//���Ͷ�������ָ��
	int write_encode = -1;
	write_encode = Write_Comm(hCom1, Encoder_Buf, 5);
	Sleep(100);

	//���յ������������
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
1�������ĵ��õ�ǰ���ǵ���Open_Comm()�ķ���ֵΪ��1�������ڴ򿪳ɹ�
2, �������ر�ʾ��ȡ������ɡ�ͽ�̤���ص�״̬����һ���ֽ����ݣ�����ת��Ϊ8λ�����ƣ�
���н�̤�����ڷ���ֵ�ĵ�8λλ��1��ʾû�в��£�0��ʾ�����ˡ�����ɡ�ڵ�7λ��1��ʾ�����˱���ɡ��0��ʾû��������
3������Encoder_chanelȡֵ��ΧΪ��1���͡�2������ʾ��������ͬ������ͨ��
********************************************************/
unsigned char Read_Protect()
{
	//��ѹ��������
	UCHAR Protect_Buf[] = { 0xff, 0x20, 0x20, 0x00, 0x00 };

	DWORD	dwSize = sizeof(Protect_Buf);
	//���Ͷ���ѹ��������ָ��
	int write_encode = -1;
	write_encode = Write_Comm(hCom2, Protect_Buf, dwSize);
	Sleep(100);
	//���յ����ѹ��������
	int read_Protect = -1;
	memset(Protect_Buf, 0, dwSize);
	read_Protect = Read_Comm(hCom2, Protect_Buf, dwSize);

	return Protect_Buf[3];
}

/*******************************************************
����������
********************************************************/
unsigned int Read_Presure(int channel)
{
	//������������
	UCHAR Presure_Buf[5] = {0};
	Presure_Buf[0] = 0xff;
	Presure_Buf[1] = 0x20;
	if (channel == 1)
		Presure_Buf[2] = 0x18;
	else
		Presure_Buf[2] = 0x19;

	//���Ͷ���ѹ��������ָ��
	int write_encode = -1;
	write_encode = Write_Comm(hCom2, Presure_Buf, 5);
	Sleep(10);
	//���յ����ѹ��������
	int read_Protect = -1;
	memset(Presure_Buf, 0, 5);
	read_Protect = Read_Comm(hCom2, Presure_Buf, 5);
	return (Presure_Buf[3] * 256 + Presure_Buf[4]);
}


/*******************************************************
1�������ĵ��õ�ǰ���ǵ���Open_Comm()�ķ���ֵΪ��1�������ڴ򿪳ɹ�
2, ��������1��ʾ����ʧ�ܣ�0��ʾ���Ƴɹ�
3������flag 1��ʾ�򿪷��ȣ�0��ʾ�رշ���
********************************************************/
int control_Fan(int flag)
{
	//������������
	UCHAR Fan_Buf[5] = {0};
	Fan_Buf[0] = 0xff;
	Fan_Buf[1] = 0x23;
	Fan_Buf[2] = 0x00;
	if (flag == 1)
		Fan_Buf[3] = 0x64;
	else
		Fan_Buf[3] = 0x00;

	Fan_Buf[4] = 0x00;

	//���Ͷ���ѹ��������ָ��
	int write_encode = -1;
	write_encode = Write_Comm(hCom2, Fan_Buf, 5);
	if (write_encode < 5)
	{
		return 1;
	}

	return 0;
}

/*******************************************************
1�����Ƶ��ƽ̨�˶�����
2, ��������1��ʾ����ʧ�ܣ�0��ʾ���Ƴɹ�
3��������ʾ��������˶���Ŀ��λ��
********************************************************/
int control_Diangang(double x, double y, double z)
{
	//��ʱ����
	return 0;
}

/***************************************************************************
**********��λ��������Ϊ�������������ɷ����˶�ƽ̨UDPЭ��������**********
****************************************************************************/
int Send_Packeg_Udp_Data(double x_Po, double y_Po, double z_Po)
{
	//��ײ�����ֵ
	const double	AccesssDistanceUnit = 400.0;
	const double	LeadDistanceUnit	= 5.0;
	const long		oneTurnPluseNum		= 10000;
	const int		AcceptIPNode		= 255;
	const int		AcceptIPGroup		= 255;
	const int		ReplyIPGroup		= 255;
	const int		ReplyIPNode			= 255;

	//����ÿ��������
	long XPulse; //���������Ӧ,1����
	long YPulse; //���������Ӧ��2����
	long ZPulse; //���������Ӧ3����
	//long UPulse; //���������Ӧ4����
	//long VPulse; //���������Ӧ5����
	//long WPulse; //���������Ӧ6����

	XPulse = x_Po / LeadDistanceUnit * oneTurnPluseNum;
	YPulse = y_Po / LeadDistanceUnit * oneTurnPluseNum;
	ZPulse = z_Po / LeadDistanceUnit * oneTurnPluseNum;
	//UPulse = AccesssDistanceUnit / LeadDistanceUnit * oneTurnPluseNum;
	//VPulse = AccesssDistanceUnit / LeadDistanceUnit * oneTurnPluseNum;
	//WPulse = AccesssDistanceUnit / LeadDistanceUnit * oneTurnPluseNum;

	//UDP��������ֵ
	const int ConfirmCode = 0x55aa; //ȷ����
	const int PassCode = 0x0000; //ͨ����
	const int FunctionCode = 0x1301; //�����룺1301����ʱ�䲥�Ź����룬1401���ʱ�䲥�Ź�����
	const int ChannelCode = 0x0000;  //0000��ƽ̨�����᣻0001��ƽ̨������

	const int WhoAcceptCode = 0xffff; //����ƽ̨������
	const int WhoReplyCode = 0xffff; //����ƽ̨��Ӧ��

	const long PlayLine = 0x00; //����UDPָ�����У�����ʱ�䲥�ſ�����Ϊ0
	const long PlayTime = 0x00; //����ʱ������Ϊ�㣬��MBOX���ư忨�ڲ�ʱ�䲥��
	const long PlayXpos = XPulse; //1�Ÿ�λ��������
	const long PlayYpos = YPulse; //2�ø�λ��������
	const long PlayZpos = ZPulse; //3�Ÿ�λ��������

	const int  BaseDoutCode = 0x1234; //12·���ֿ��������,��12λ��Ч

	const int  DacOneCode = 0x5678; //ģ����1�����
	const int  DacTwoCode = 0xabcd; //ģ����2�����

	//������UDO���ݵ�����
	char Buffer[49];
	char UDPBuffer[49];

	//��˫�ֺ��ֳַ��ֽڣ�����λ��ǰ��λ�ں�
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

/*��ʼ��*/
BOOL Init()
{
	//��ʼ�������
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

/*��ʼ�������*/
BOOL StartLaJYJ()
{
	BOOL bRet = FALSE;
	
	//˳���ܵ�
	UCHAR byCmd1[] = { 0xff, 0x23, 0x03, 0x64, 0x00 };
	UCHAR byCmd2[] = { 0xff, 0x23, 0x01, 0x64, 0x00 };

	Write_Comm(hCom2, byCmd1, 5);
	Sleep(JUANJI_SLEEP_TIME);
	Write_Comm(hCom2, byCmd2, 5);

	return bRet;
}

//ֹͣ�������
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

/*��ʼ�ž����*/
BOOL StartFangJYJ()
{
	BOOL bRet = FALSE;

	//˳���ܵ�
	UCHAR byCmd1[] = { 0xff, 0x23, 0x02, 0x64, 0x00 };
	UCHAR byCmd2[] = { 0xff, 0x23, 0x01, 0x64, 0x00 };

	Write_Comm(hCom2, byCmd1, 5);
	Sleep(JUANJI_SLEEP_TIME);
	Write_Comm(hCom2, byCmd2, 5);

	return bRet;
}

//ֹͣ�ž����
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