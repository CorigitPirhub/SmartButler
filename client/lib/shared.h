#ifndef UDP_CLIENT_TEST_H
#define UDP_CLIENT_TEST_H

// 函数声明
void UdpClientTest(const char* host, unsigned short port);

// 发送UDP请求的值
extern char request[128];

// 全局变量声明
extern char HX_VALUE;   // 红外遥控的值
extern float global_temp;   // 温度
extern float global_humi;   // 湿度

// 服务器的IP地址
extern char CLIENT_HOST[128];
extern char RGB_HOST[128];
extern char DIM_HOST[128];
extern char DOOR_HOST[128];

#endif // UDP_CLIENT_TEST_H
