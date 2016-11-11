#ifndef LIB_H
#define LIB_H
/******************************************************************
************************电缸控制接口*****************************
*******************************************************************/
extern "C" int __declspec(dllexport) Send_Packeg_Udp_Data(double x_Po,double y_Po,double z_Po);
extern "C" int __declspec(dllexport) Send_Packeg_Udp_Data_relative(double x_Po, double y_Po, double z_Po, long Time);

extern "C" int __declspec(dllexport) Creat_UDP();
extern "C" int __declspec(dllexport) add(int,int);






//////////////////////////////////////////////



#endif