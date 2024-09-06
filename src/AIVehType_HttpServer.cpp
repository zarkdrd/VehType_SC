/**************************************************
     >Author: zarkdrd
     >Date: 2024-09-04 15:52:07
     >LastEditTime: 2024-09-05 14:43:11
     >LastEditors: zarkdrd
     >Description:
     >FilePath: /VehType_SC/src/AIVehType_HttpServer.cpp
**************************************************/
#include "AIVehType_HttpServer.h"
#include "AIVehTypeMng.h"
#include "Log_Message.h"
#include "VehType.h"
#include "AIVehTypeMng.h"
#include "GetTime.h"
#include "convert.h"
#include "VehType.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

using std::string;

extern int g_upload_mode;

AIVT_RequestHandLer::AIVT_RequestHandLer()
{
    m_pregBuff = new char[BUFLEN];
    // 插入键值对，在容器中用键查找值，实现转换
    qmColorNo.insert(std::pair<int, string>(0, "其它"));
    qmColorNo.insert(std::pair<int, string>(1, "黑"));
    qmColorNo.insert(std::pair<int, string>(2, "白"));
    qmColorNo.insert(std::pair<int, string>(3, "蓝"));
    qmColorNo.insert(std::pair<int, string>(4, "黄"));
    qmColorNo.insert(std::pair<int, string>(5, "绿"));
    qmColorNo.insert(std::pair<int, string>(6, "黄绿"));

    qmVehTypeNo.insert(std::pair<string, string>("客一", "1"));
    qmVehTypeNo.insert(std::pair<string, string>("客二", "2"));
    qmVehTypeNo.insert(std::pair<string, string>("客三", "3"));
    qmVehTypeNo.insert(std::pair<string, string>("客四", "4"));
    qmVehTypeNo.insert(std::pair<string, string>("货一", "11"));
    qmVehTypeNo.insert(std::pair<string, string>("货二", "12"));
    qmVehTypeNo.insert(std::pair<string, string>("货三", "13"));
    qmVehTypeNo.insert(std::pair<string, string>("货四", "14"));
    qmVehTypeNo.insert(std::pair<string, string>("货五", "15"));
    qmVehTypeNo.insert(std::pair<string, string>("货六", "16"));
    qmVehTypeNo.insert(std::pair<string, string>("专一", "21"));
    qmVehTypeNo.insert(std::pair<string, string>("专二", "22"));
    qmVehTypeNo.insert(std::pair<string, string>("专三", "23"));
    qmVehTypeNo.insert(std::pair<string, string>("专四", "24"));
    qmVehTypeNo.insert(std::pair<string, string>("专五", "25"));
    qmVehTypeNo.insert(std::pair<string, string>("专六", "26"));
}

void AIVT_RequestHandLer::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
{
    // string data(std::istreambuf_iterator<char>(request.stream()));
    std::istreambuf_iterator<char> eos;
    std::string data(std::istreambuf_iterator<char>(request.stream()), eos);
    // std::cout<<"request data:"<<data<<std::endl;
    //  std::cout << "URL:" << request.getURI() << std::endl;
    // log_message(INFO,"VehType URL:%s",request.getURI().c_str());
    //  memset(m_pregBuff, 0, BUFLEN);
    //  std::cout << "m_pregBuff:" << m_pregBuff << std::endl;
    if (request.getURI() == "/RegResult" || request.getURI() == "/VehVideo")
    {
        handleRegResult(data);
    }
    else if (request.getURI() == "/HeartBeat")
    {
        handleHeartBeat(data);
    }
    // else if (request.getURI() == "/VehVideo")
    // {
    //     handleVideoResult(data);
    // }

    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    response.setChunkedTransferEncoding(true);                 // 设置分组传输编码
    response.setContentType("application/json;charset=UTF-8"); // 设置内容类型
    response.send();
}

void AIVT_RequestHandLer::handleRegResult(std::string &data)
{
    // 响应参数结构体
    VehType_Info info;
    Poco::JSON::Parser jsonParser;
    log_message(INFO, "handleRegResult:");
    Poco::JSON::Object::Ptr object = jsonParser.parse(data).extract<Poco::JSON::Object::Ptr>();
    // Poco::Dynamic::Var datapParams = object->get("params");
    // Poco::JSON::Object::Ptr objParams = datapParams.extract<Poco::JSON::Object::Ptr>();

    uint8_t dataRelayFlag = 0;
    // 车头车牌号码
    if (object->get("plateNo").isString())
    {
        info.vehPlate = object->get("plateNo").toString();
        dataRelayFlag |= 0x01;
        std::cout << "info.vehPlate:" << info.vehPlate << std::endl;
        // log_message(INFO,"info.vehPlate:%s",info.vehPlate.c_str());
    }

    // 车头车牌颜色
    if (object->get("plateColor").isString())
    {
        string sColorNo = object->get("plateColor").toString();
        int nColorNo = atoi(sColorNo.c_str());
        if (qmColorNo.find(nColorNo) != qmColorNo.end())
        {
            info.vehPlateColor = qmColorNo[nColorNo];
        }
        else
        {
            info.vehPlateColor = "未知";
        }
        dataRelayFlag |= 0x02;
        std::cout << "info.vehPlateColor:" << info.vehPlateColor << std::endl;
        // log_message(INFO,"info.vehPlateColor:%s",info.vehPlateColor.c_str());
    }

    // 收费车型
    if (object->get("vehicleType").isString())
    {
        string sVehicleType = object->get("vehicleType").toString();
        // char buf[8] = {0};
        // charset_convert_UTF8_TO_GBK((char *)sVehicleType.c_str(), strlen(sVehicleType.c_str()), buf, 8);
        // std::cout << "sVehicleType:" << sVehicleType << std::endl;
        if (qmVehTypeNo.find(sVehicleType) != qmVehTypeNo.end())
        {
            info.vehType = qmVehTypeNo[sVehicleType];
        }
        else
        {
            info.vehType = "0"; // 未知车型
        }
        dataRelayFlag |= 0x04;
        std::cout << "info.vehType:" << info.vehType << std::endl;
        // log_message(INFO,"info.vehType:%s",info.vehType.c_str());
    }

    // 轴数
    if (object->get("axleCount").isNumeric())
    {
        info.axleCount = to_string(object->getValue<int>("axleCount"));
        dataRelayFlag |= 0x08;
        std::cout << "info.axleCount:" << atoi(info.axleCount.c_str()) << std::endl;
        // log_message(INFO,"info.axleCount:%d",atoi(info.axleCount.c_str()));
    }

    // 置信度
    if (object->get("confidence").isString())
    {
        info.confidence = object->get("confidence").toString();
        // 1. 将字符串转换为浮点数
        double confidence = std::atof(info.confidence.c_str());
        // 2. 乘以 100
        confidence *= 100;
        // 3. 将结果转换回字符串
        std::ostringstream oss;
        oss << confidence;
        std::string confidence_percentage = oss.str();
        info.confidence = confidence_percentage;

        // std::cout << "info.confidence:" << info.confidence << std::endl;
        log_message(INFO, "info.confidence:%s", info.confidence.c_str());
    }

    // 序号
    if (object->get("id").isString())
    {
        info.id = object->get("id").toString();
        log_message(INFO, "info.id=%s", info.id.c_str());
    }

    // 上传数据时间，用于后续监测删除
    time_t current_time = time(NULL);
    info.nDelTime = current_time;

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
    info.timestamp = timestamp_stream.str();

    log_message(INFO, "接收到车型数据 车牌:%s 车牌颜色：%s 车型:%s 轴数：%s", info.vehPlate.c_str(), info.vehPlateColor.c_str(), info.vehType.c_str(), info.axleCount.c_str());
    VehType *myVehType = new VehType;
    if (dataRelayFlag == 0x0F)
    {
        // 判断是否为主动模式/等待模式
        if (g_upload_mode == 1) // 主动上传
        {
            myVehType->VehTypeResultReport(info, 0x01); // 向上位机发送上报帧
        }
        else if (g_upload_mode == 2) // 等待指令后再上传
        {
            // 上传数据到链表
            AIVehTypeMng::Instance().PushRegResult(info);
        }
    }
    else
    {
        myVehType->VehTypeResultReport(info, 0x02);
        log_message(ERROR, "车型设备接收数据不全,dataRelayFlag=%02X", dataRelayFlag);
    }

    info.clearData();
}

std::string AIVT_RequestHandLer::currentDateToString()
{
    // static int i = 100000;
    // i++;
    // return to_string(i);
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80] = {0};

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", timeinfo);
    std::string str(buffer);
    return str;
}

void AIVT_RequestHandLer::handleHeartBeat(std::string &data)
{
    // AIVT_RequestHandLer::GetInstance().UpdateHeartTime();
}

void AIVT_RequestHandLer::handleVideoResult(std::string &data)
{

    Poco::JSON::Parser jsonParser;
    // std::cout << "handleVideoResult" << std::endl;
    log_message(INFO, "handleVideoResult");
    Poco::JSON::Object::Ptr object = jsonParser.parse(data).extract<Poco::JSON::Object::Ptr>();
}

HTTPRequestHandler *RequestHandlerFactory::createRequestHandler(const HTTPServerRequest &request)
{
    return new AIVT_RequestHandLer();
}
