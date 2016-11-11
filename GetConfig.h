/**************************************************************************** 
*   日期:  2014-12-5 
*   目的:  读取配置文件的信息，以map的形式存入 
*   要求:  配置文件的格式，以#作为行注释，配置的形式是key = value，中间可有空 
格，也可没有空格 
*****************************************************************************/  
      
#ifndef _GET_CONFIG_H_  
#define _GET_CONFIG_H_  

#include <string>  
#include <map>

#define COMMENT_CHAR '#'
/*配置信息*/
#define CONFIG_FILE			"init.conf"			//配置文件名
#define IPADDR_PILOT		"ip_addr_polit"		//投放员和udpclient之间用的虚拟网卡IP地址
#define IPADDR				"ip_address"		//fg和udpclient之间用的虚拟网卡IP地址
#define PORT_FG2APP			"port_fg2app"		//fg给udpclient开放的udp服务端口，udpclient向fg发送数据时用
#define PORT_APP2FG			"port_app2fg"		//udpclient给fg开放的udp服务端口
#define DG_IPADDR			"dg_ipaddr"			//电缸开放的IP地址
#define PORT_DG2APP			"port_dg2app"		//电缸开放的端口
#define PORT_COM1			"port_com1"			//控制拉绳箱
#define PORT_COM2			"port_com2"			//控制风扇，脚踏，备份伞，拉力传感，卷扬机
#define DEBUG_NO_HARDWARE	"debug_no_hardware"	//在没有硬件的情况下进行调试,1代表有调试，0代表没有调试

using namespace std;  
      
bool ReadConfig(const string & filename, map<string, string> & m);  
void PrintConfig(const map<string, string> & m);  
      
#endif  