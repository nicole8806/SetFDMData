#ifndef CENTER_CON
#define CENTER_CON

/***Ӳ����ʼ���������رվ���������ȵ�Ӳ�����أ���֤��ȫʹ��***/
extern "C" int __declspec(dllexport) init_hardware();

/***flag�������壺0ֹͣ�����ת����1������ţ�2�������.time���ƾ�������˶�ʱ�䣬��λ������ms***/
extern "C" int __declspec(dllexport) Control_Juanyangji(int flag,int time);

/***flag�������壺0ֹͣ�����ת����1�������3�룬2�������3��***/
/**ע�⣺�˺������ƾ�����˶��󣬱��봫��ʵ��0�ٴε��ò���ֹͣ������˶�**/
extern "C" int __declspec(dllexport) Control_Juanyangji_info(int flag);

/***flag�������壺0ֹͣ����ת����1���ȿ�ʼת��***/
extern "C" int __declspec(dllexport) Control_Fengshan(int flag);

/***��ȡ��̤�壬����ɡ״ֵ̬����������һ���ֽ����ݣ�
��̤���ڵ�5λ����0����ʾ��̤����£���1����ʾ��̤��û�в���
����ɡ�ڵ�7λ����0����ʾ����ɡû�д򿪣���1����ʾ����ɡ��***/
extern "C" unsigned char __declspec(dllexport) Get_Protect(void);

/***��ȡ���ݴ�����ֵ***/
/*****flag����1����ʾ��ȡ��߲��ݴ����ݣ�2��ʾ��ȡ�ұ߲��ݴ�����***********/
extern "C" int __declspec(dllexport) Get_Tension_Data(int flag);

/***��ȡ���ݰ�����ֵ***/
/*****flag����1����ʾ��ȡ��ǰ�������ݣ�2��ʾ��ȡ�Һ��������ݣ�3��ʾ��ȡ����������ݣ�4��ʾ��ȡ��ǰ��������***********/
extern "C" int __declspec(dllexport) Get_Pull_Data(int flag);

//���Խӿڣ����ڴ����������ԣ���Ӳ���޹�
extern "C" int __declspec(dllexport)decrease(int ,int);
//extern "C" int __declspec(dllexport) add(int,int);
#endif