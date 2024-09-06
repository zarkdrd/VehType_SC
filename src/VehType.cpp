/**************************************************
     >Author: zarkdrd
     >Date: 2024-09-04 15:54:20
     >LastEditTime: 2024-09-05 16:14:58
     >LastEditors: zarkdrd
     >Description:
     >FilePath: /VehType_SC/src/VehType.cpp
**************************************************/
#include "VehType.h"
#include "Get_Inifiles.h"
#include "TcpServer.h"
#include "AIVehType_HttpServer.h"
#include "AIVehType_Interface.h"
#include "Log_Message.h"
#include "DataCheck.h"
#include "cJSON.h"
#include "convert.h"

/********************************【配置与初始化】****************************************/
VehType::VehType() {
};

VehType::~VehType()
{
     delete myTcpServer;
}

bool VehType::ReadConfig()
{
     log_message(INFO, "获取配置文件信息...");
     myTcpServer = new TcpServer(IniFile::getInstance()->GetIntValue("Tcp", "DevicePort", 9528));
     log_message(INFO, "获取配置文件已完成！");
     return true;
}

bool VehType::Init()
{
     ReadConfig();
     log_message(INFO, "正在初始化车型识别设备...");

     // 此处启动各个线程
     if (tcpserverThread.joinable() == false)
     {
          log_message(INFO, "此时服务器线程已经启动,正在等待上位机分配IP等重要参数...");
          myTcpServer->Open();
          tcpserverThread = std::thread(TcpServerThread, this); // 程序对车道软件通讯的服务器线程
     }

     if (xiaoshentongThread.joinable() == false)
     {
          xiaoshentongThread = std::thread(http_server, this); // 对接小神瞳系统http服务器的线程
          log_message(INFO, "对接小神瞳系统的服务器线程已经启动！");
     }
     log_message(INFO, "初始化车型识别设备已完成,所有线程已经启动！");

     return true;
}

bool VehType::Start()
{
     while (1)
     {
          if (myTcpServer->isOpen == false)
          {
               // log_message(INFO, "Tcp服务端现在为关闭状态!");
               sleep(1);
               continue;
          }
          sleep(10);
     }
     return true;
}

/***************************************【OVER】**********************************************/
/**************************************【线程区】*********************************************/
// 服务器线程
void TcpServerThread(class VehType *myVehType)
{
     while (1)
     {
          if (myVehType->myTcpServer->isOpen == false)
          {
               usleep(100 * 1000);
               continue;
          }
          myVehType->ServerReceiveCmd();
     }
}

/***************************************【OVER】**********************************************/
/**************************************【指令帧区】********************************************/
// 设备开关响应帧
int VehType::DeviceOpen()
{
     int datalen = 0;
     char response_data[1024];
     char json_data[1024];
     int Type = 0xB1;
     sprintf(json_data,
             "{"
             "\"result\":\"%s\","
             "\"errMsg\":\"%s\","
             "}",
             "1", "errMsg:NULL");
     memcpy(&response_data, json_to_hex(json_data), strlen(json_data) * 2);
     datalen += strlen(json_data) * 2;

     Communication((unsigned char *)response_data, datalen, Type);
     return 0;
}

// 车型结果上传响应帧
int VehType::VehTypeResultReport(VehType_Info info, int Mode)
{
     int datalen = 0;
     char response_data[1024];
     char json_data[1024];
     int Type = Mode = AOTOUPLOAD ? 0xB2 : 0xB3;

     // 设置车牌颜色
     int vehColorNum = -1;
     if (info.vehPlateColor == "蓝")
     {
          vehColorNum = 0;
     }
     else if (info.vehPlateColor == "黄")
     {
          vehColorNum = 1;
     }
     else if (info.vehPlateColor == "黑")
     {
          vehColorNum = 2;
     }
     else if (info.vehPlateColor == "白")
     {
          vehColorNum = 3;
     }
     else if (info.vehPlateColor == "绿")
     {
          vehColorNum = 4;
     }
     else if (info.vehPlateColor == "黄绿")
     {
          vehColorNum = 5;
     }
     else
     {
          log_message(ERROR, "颜色未知,发送失败。info.vehColor:%s", info.vehPlateColor.c_str());
          return -1;
     }
     sprintf(json_data,
             "{"
             "\"result\":\"%s\","
             "\"vlpTime\":\"%s\","
             "\"vehNo\":\"%s\","
             "\"vlpReliability\":\"%s\","
             "\"vlpText\":\"%s\","
             "\"vlpColor\":\"%s\","
             "\"vehClass\":\"%s\","
             "\"vehAxis\":\"%s\""
             "}",
             "1", info.timestamp, info.id, info.confidence, info.vehPlate, vehColorNum, info.vehType, info.axleCount);

     memcpy(&response_data, json_to_hex(json_data), strlen(json_data) * 2);
     datalen += strlen(json_data) * 2;

     Communication((unsigned char *)response_data, datalen, Type);
     return 0;
}

// 心跳响应帧
int VehType::HeartBeatReport()
{
     int datalen = 0;
     char response_data[1024];
     char json_data[1024];
     int Type = 0xB4;

     // 获取当前系统时间，精确到秒
     auto now = std::chrono::system_clock::now();
     auto time_in_seconds = std::chrono::system_clock::to_time_t(now);
     // 将时间转换为 struct tm 格式
     struct tm *tm_info = localtime(&time_in_seconds);
     // 获取当前时间的毫秒部分
     auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
     // 格式化时间为 yyyyMMddHHmmss
     char buffer[20];
     strftime(buffer, 20, "%Y%m%d%H%M%S", tm_info);
     // 拼接毫秒部分 zzz
     std::ostringstream timestamp_stream;
     timestamp_stream << buffer << std::setw(3) << std::setfill('0') << milliseconds.count();
     std::string timestamp = timestamp_stream.str();

     // 2. 读取系统的运行时长（uptime）
     std::ifstream uptime_file("/proc/uptime");
     double uptime_seconds = 0;
     uptime_file >> uptime_seconds;
     uptime_file.close();
     // 3. 用当前时间减去系统运行时间，得到设备启动时间
     time_t start_time_in_seconds = time_in_seconds - static_cast<time_t>(uptime_seconds);
     // 4. 将设备启动时间转换为 struct tm
     struct tm *tm_info1 = localtime(&start_time_in_seconds);
     // 5. 获取设备启动时间的毫秒部分（假设为 000，启动时间精确到秒）
     char buffer1[20];
     strftime(buffer1, 20, "%Y%m%d%H%M%S", tm_info1);
     // 6. 拼接毫秒部分 zzz（假设为 000，因为启动时间是秒级的）
     std::ostringstream timestamp_stream1;
     timestamp_stream1 << buffer1 << "000"; // 启动时间没有毫秒精度，默认 "000"
     std::string startTime = timestamp_stream1.str();

     sprintf(json_data,
             "{"
             "\"dataTime\":\"%s\","
             "\"vID\":\"%s\","
             "\"devVer\":\"%s\","
             "\"startTime\":\"%s\","
             "\"errMsg\":\"%s\","
             "\"spare1\":\"%s\","
             "\"spare2\":\"%s\","
             "\"spare3\":\"%s\","
             "\"result\":\"%s\""
             "}",
             timestamp, "ARTC", "V1.00", startTime, "errMsg:NULL", "spare1", "spare2", "spare3", "0");
     memcpy(&response_data, json_to_hex(json_data), strlen(json_data) * 2);
     datalen += strlen(json_data) * 2;

     Communication((unsigned char *)response_data, datalen, Type);
     return 0;
}

// 与车道软件通讯
int VehType::Communication(unsigned char *Oput, int OputLen, int Type)
{
     int DataLen = 0;
     unsigned char SendData[1024];
     unsigned int CRC = 0;

     SendData[DataLen++] = 0xFF; // 帧开始的标志
     SendData[DataLen++] = 0xFF;

     SendData[DataLen++] = 0x00; // 协议版本号

     SendData[DataLen++] = Type; // 指令代码

     SendData[DataLen++] = 0x00; // DATA 域的长度，4 字节
     SendData[DataLen++] = 0x00; // 高 2 字节保留，低两字节为 DATA 域长度
     SendData[DataLen++] = OputLen / 256;
     SendData[DataLen++] = OputLen % 256;

     memcpy(&SendData[DataLen], &Oput[0], OputLen); // 帧数据内容
     DataLen += OputLen;

     CRC = CRC16(&SendData[2], DataLen - 2); // CRC校验
     SendData[DataLen++] = CRC / 256;
     SendData[DataLen++] = CRC % 256;

     SendData[DataLen++] = 0xFF; // 帧结束标志

     if (myTcpServer->isOpen == false)
     {
          return -1;
     }
     else
     {
          myTcpServer->WriteByte(SendData, DataLen);
          hex_message(INFO, "向车道软件发送数据帧:", SendData, DataLen);
     }

     return 0;
}
/***************************************【OVER】**********************************************/
/**************************************【数据解析区】******************************************/
// 解析数据具体功能
int VehType::Analysis(unsigned char *Cmd, int CmdLen)
{
     char json_data[1024];
     memcpy(&json_data, hex_to_json((char *)Cmd[8]), (CmdLen - 11) / 2);
     switch (Cmd[3])
     {
     case 0xC1:
          DeviceOpen();
          log_message(INFO, "对设备进行了开关操作。");
          break;

     case 0xC3:
          log_message(INFO, "收到手动查询车型识别");
          break;

     case 0xC4:
          HeartBeatReport();
          log_message(INFO, "收到了车道软件心跳帧");
          break;

     default:
          log_message(WARN, "未知的数据帧！");
          break;
     }
     return 0;
}

// 解析数据头与crc校验
bool VehType::ServerReceiveCmd()
{
     int i;
     int ret, Index1 = -1, Index2 = 0, pos = 0;
     int Command_Len, Frame_Data_Len;
     unsigned char Recv_Data_Min[2048];
     unsigned char Recv_Data[2048];
     unsigned char Recv_Data_bak[2048];

     while (1)
     {
          ret = myTcpServer->RecvByte(Recv_Data_Min, sizeof(Recv_Data_Min));
          hex_message(INFO, "从客户端接收到信息:", Recv_Data_Min, sizeof(Recv_Data_Min));
          if (ret <= 0)
          {
               // log_message(ERROR, "recv 返回值 [%d] 错误描述 [%s]", ret, strerror(errno));
               // log_message(ERROR, "客户端断开连接，等待重连...");
               myTcpServer->Close();
               return false;
          }

          /**将获取到的数据进行保存**/
          memcpy(Recv_Data + pos, Recv_Data_Min, ret);
          pos += ret;
          if (pos < 2)
          {
               continue;
          }
     Find_Frame:
          for (i = Index2; i < pos - 1; i++)
          {
               /**找到数据头**/
               if ((Recv_Data[i] == 0xFF) && (Recv_Data[i + 1] == 0xFF) && (Recv_Data[i + 2] == 0x00))
               {
                    Index1 = i;
                    break;
               }
          }
          if (Index1 < 0)
          {
               /**未找到数据头，清空数据**/
               Index2 = 0;
               pos = 0;
               continue;
          }

          if (pos - Index1 < 11)
          {
               continue;
          }
          else
          {
               /**获取指令长度**/
               Command_Len = Recv_Data[Index1 + 6] * 256 + Recv_Data[Index1 + 7];
          }

          if ((Command_Len + 11) > (pos - Index1))
          {
               log_message(INFO, "数据帧不完整");
               /**数据帧不完整**/
               continue;
          }
          else if ((Command_Len + 11) < (pos - Index1))
          {
               log_message(INFO, "数据帧大于一帧");
               /**大于一帧数据**/
               Frame_Data_Len = Command_Len + 11;
               Index2 = Index1 + Frame_Data_Len;
               printf("framelen:%d\n", Frame_Data_Len);
               printf("command:%d\n", Command_Len);
               printf("pos - Index1:%d\n", pos - Index1);
          }
          else
          {
               log_message(INFO, "数据帧刚好一帧");
               /**刚好一帧数据**/
               Index2 = 0;
               Frame_Data_Len = Command_Len + 11;
               printf("framelen:%d\n", Frame_Data_Len);
               printf("command:%d\n", Command_Len);
          }
          unsigned int crc = CRC16(&Recv_Data[Index1 + 2], Frame_Data_Len - 5);
          char buf[2] = {0};
          buf[0] = (crc >> 8) & 0xFF;
          buf[1] = crc & 0xFF;
          printf("buf0:%2X\n", buf[0]);
          printf("buf1:%2X\n", buf[1]);
          if ((((crc >> 8) & 0xFF) == Recv_Data[Index1 + Frame_Data_Len - 2]) || ((crc & 0xFF) == Recv_Data[Index1 + Frame_Data_Len - 1]))
          {
               Analysis(&Recv_Data[Index1], Frame_Data_Len); // 传入数据帧与数据帧长度
          }
          else
          {
               /**说明这个数据帧是错的，找第二个头**/
               hex_message(ERROR, "Recv Tcp data err:", &Recv_Data[Index1], Frame_Data_Len);
               for (i = Index1 + 1; i < pos - 1; i++)
               {
                    /**找到数据头**/
                    if (Recv_Data[i] == 0xFF && Recv_Data[i + 1] == 0xFF)
                    {
                         Index1 = -1;
                         Index2 = i;
                         goto Find_Frame;
                    }
               }
               Index2 = 0;
               pos = 0;
               continue;
          }

          if (Index2 == 0)
          {
               /**无拼帧情况**/
               pos = 0;
               Index1 = -1;
               Index2 = 0;
               memset(Recv_Data, 0, sizeof(Recv_Data));
               continue;
          }

          if (Index2 > 0)
          {
               /**有拼帧情况**/
               pos = pos - Index2;
               memcpy(Recv_Data_bak, Recv_Data + Index2, pos);
               memset(Recv_Data, 0, sizeof(Recv_Data));
               memcpy(Recv_Data, Recv_Data_bak, pos);
          }

          if (pos < 4)
          {
               /**不够完整的帧**/
               Index1 = -1;
               Index2 = 0;
               continue;
          }
          if ((Recv_Data[2] + 4) > pos)
          {
               /**帧长度不够**/
               Index1 = -1;
               Index2 = 0;
               continue;
          }
          /**帧长度充足，再次解析帧**/
          Index1 = -1;
          Index2 = 0;
          goto Find_Frame;
     }
}

/***************************************【OVER】**********************************************/