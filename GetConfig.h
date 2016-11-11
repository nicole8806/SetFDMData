/**************************************************************************** 
*   ����:  2014-12-5 
*   Ŀ��:  ��ȡ�����ļ�����Ϣ����map����ʽ���� 
*   Ҫ��:  �����ļ��ĸ�ʽ����#��Ϊ��ע�ͣ����õ���ʽ��key = value���м���п� 
��Ҳ��û�пո� 
*****************************************************************************/  
      
#ifndef _GET_CONFIG_H_  
#define _GET_CONFIG_H_  

#include <string>  
#include <map>

#define COMMENT_CHAR '#'
/*������Ϣ*/
#define CONFIG_FILE			"init.conf"			//�����ļ���
#define IPADDR_PILOT		"ip_addr_polit"		//Ͷ��Ա��udpclient֮���õ���������IP��ַ
#define IPADDR				"ip_address"		//fg��udpclient֮���õ���������IP��ַ
#define PORT_FG2APP			"port_fg2app"		//fg��udpclient���ŵ�udp����˿ڣ�udpclient��fg��������ʱ��
#define PORT_APP2FG			"port_app2fg"		//udpclient��fg���ŵ�udp����˿�
#define DG_IPADDR			"dg_ipaddr"			//��׿��ŵ�IP��ַ
#define PORT_DG2APP			"port_dg2app"		//��׿��ŵĶ˿�
#define PORT_COM1			"port_com1"			//����������
#define PORT_COM2			"port_com2"			//���Ʒ��ȣ���̤������ɡ���������У������
#define DEBUG_NO_HARDWARE	"debug_no_hardware"	//��û��Ӳ��������½��е���,1�����е��ԣ�0����û�е���

using namespace std;  
      
bool ReadConfig(const string & filename, map<string, string> & m);  
void PrintConfig(const map<string, string> & m);  
      
#endif  