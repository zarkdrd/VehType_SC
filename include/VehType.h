/**************************************************
     >Author: zarkdrd
     >Date: 2024-09-04 15:54:32
     >LastEditTime: 2024-09-05 15:59:19
     >LastEditors: zarkdrd
     >Description:
     >FilePath: /VehType_SC/include/VehType.h
**************************************************/
#ifndef _INCLUDE_VEHTYPE_H_
#define _INCLUDE_VEHTYPE_H_

#include <thread>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <string>
#include <cmath>
#include <mutex>
#include <unistd.h> // For sleep()
#include <iostream>

#include "AIVehTypeMng.h"

#define AOTOUPLOAD 0x01
#define MANUALUPLOAD 0x02
class VehType
{
public:
     VehType();
     ~VehType();
     bool ReadConfig();                                             // 读配置文件
     bool Init();                                                   // 初始化
     bool Start();                                                  // 启动
     int DeviceOpen();                                              // 开关设备
     int VehTypeResultReport(VehType_Info info, int Mode);          // 车型结果上传
     int Communication(unsigned char *Oput, int OputLen, int Type); // 与车道软件通讯
     int HeartBeatReport();                                         // 心跳
     int Analysis(unsigned char *Cmd, int CmdLen);                  // 解析数据具体内容
     bool ServerReceiveCmd();                                       // 服务器接收数据

public:
     class TcpServer *myTcpServer;

private:
     std::thread tcpserverThread;
     std::thread xiaoshentongThread;

public:
     int DeviceOpenFlag; // 设备开关标志
};

void TcpServerThread(class VehType *myVehType);

#endif // !_INCLUDE_VEHTYPE_H_
