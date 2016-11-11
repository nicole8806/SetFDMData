#ifndef CENTER_CON
#define CENTER_CON

/***硬件初始化函数，关闭卷扬机，风扇等硬件开关，保证安全使用***/
extern "C" int __declspec(dllexport) init_hardware();

/***flag参数意义：0停止卷扬机转动，1卷扬机放，2卷扬机缩.time控制卷扬机的运动时间，单位：毫秒ms***/
extern "C" int __declspec(dllexport) Control_Juanyangji(int flag,int time);

/***flag参数意义：0停止卷扬机转动，1卷扬机拉3秒，2卷扬机缩3秒***/
/**注意：此函数控制卷扬机运动后，必须传入实参0再次调用才能停止卷扬机运动**/
extern "C" int __declspec(dllexport) Control_Juanyangji_info(int flag);

/***flag参数意义：0停止风扇转动，1风扇开始转动***/
extern "C" int __declspec(dllexport) Control_Fengshan(int flag);

/***获取脚踏板，备份伞状态值，函数返回一个字节数据，
脚踏板在第5位，‘0’表示脚踏板踩下，‘1’表示脚踏板没有踩下
备份伞在第7位，‘0’表示备份伞没有打开，‘1’表示备份伞打开***/
extern "C" unsigned char __declspec(dllexport) Get_Protect(void);

/***获取操纵带拉力值***/
/*****flag参数1，表示获取左边操纵带数据，2表示获取右边操纵带数据***********/
extern "C" int __declspec(dllexport) Get_Tension_Data(int flag);

/***获取操纵棒拉绳值***/
/*****flag参数1，表示获取右前拉绳数据，2表示获取右后拉绳数据，3表示获取左后拉绳数据，4表示获取左前拉绳数据***********/
extern "C" int __declspec(dllexport) Get_Pull_Data(int flag);

//测试接口，用于此软件自身测试，与硬件无关
extern "C" int __declspec(dllexport)decrease(int ,int);
//extern "C" int __declspec(dllexport) add(int,int);
#endif