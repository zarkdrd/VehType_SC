/**************************************************
     >Author: zarkdrd
     >Date: 2024-08-16 15:08:48
     >LastEditTime: 2024-09-05 14:38:54
     >LastEditors: zarkdrd
     >Description:
     >FilePath: /VehType_SC/include/AIVehTypeMng.h
**************************************************/
#ifndef _INCLUDE_AIVEHTYPEMNG_H_
#define _INCLUDE_AIVEHTYPEMNG_H_

#include "Datadef.h"
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/JSON/Array.h>
#include <list>
#include <string>

using namespace std;
using Poco::Net::HTTPServer;

#define PLATE 1
#define GUID 2

// 上传信息
typedef struct RESPONSE_INFO_FORJSON
{
    string checkTime;     // checkTime  String  检测时间
    string vehPlate;      // vehPlate	String	车牌号
    string vehPlateColor; // vehPlateColor String 车牌颜色
    string vehType;       // vehType	String	车型
    string axleCount;     // axleCount  String  轴数
    string confidence;    // 车牌置信度
    string id;            // 序号
    string guid;          // guid       String  车型识别设备车辆信息标识
    string timestamp;     // timestamp

    time_t nDelTime;

    RESPONSE_INFO_FORJSON() = default; // 默认构造函数

    RESPONSE_INFO_FORJSON &operator=(const RESPONSE_INFO_FORJSON &other)
    {
        if (this != &other)
        { // 检查是否为自我赋值
            checkTime = other.checkTime;
            vehPlate = other.vehPlate;
            vehPlateColor = other.vehPlateColor;
            vehType = other.vehType;
            axleCount = other.axleCount;
            confidence = other.confidence;
            id = other.id;
            guid = other.guid;

            nDelTime = other.nDelTime;
            return *this;
        }

        return *this;
    }

    void clearData()
    {
        *this = RESPONSE_INFO_FORJSON(); // 创建一个新实例并赋值给当前实例
    }

} VehType_Info;

typedef struct
{
    time_t nDelTime;        // 删除时间
    VehType_Info stVehInfo; // 车型信息
} SVehDel;

// AIVehTypeMng类，用于管理AIVehTypeMng实例
class AIVehTypeMng
{
public:
    // AIVehTypeMng(const AIVehTypeMng&) = delete;
    // AIVehTypeMng& operator=(const AIVehTypeMng&) = delete;
    static AIVehTypeMng &Instance()
    {
        static AIVehTypeMng instance;
        return instance;
    }

    // 初始化AIVehTypeMng实例
    int Init();

    // 处理接收的tcp数据
    static void DealRecvTcpDataThread();

    // 启动http服务器
    int StartHttpServer();

    // 停止http服务器
    void StopHttpServer();

    // 设置摄像头IP地址
    void SetCameraIP(std::string sCameraIP);

    // 获取设备号
    int GetDeviceNo();

    // 设置回调函数
    void SetCallBack(pAIVTCb AIVTCb);

    // 上传结果(从队列获取车辆信息)
    VehType_Info UploadResult(string param, int method);

    // 获取队列头部的车辆信息
    VehType_Info getHeadVehicleInfo();

    // 获取所有车型信息
    Poco::JSON::Array getAllVehicleInfo();

    // 获取状态
    int GetState();

    // 将注册结果放入队列
    void PushRegResult(VehType_Info &stVehInfo);

    // // 调用回调函数
    // void InvokeCallBack(VehType_Info *pVehicleInfo);

    // 清除超时列表
    void ClearTimeoutList();

private:
    AIVehTypeMng();
    ~AIVehTypeMng();

    // 读取ini配置文件
    void readIniCfg();
    // 根据车牌号获取车型信息
    VehType_Info getVehicleInfoFromPlate(string strPlateNo, int method);
    // 根据车牌号获取车型信息
    VehType_Info getVehicleInfoFromDelList(string strPlateNo, int method);
    // 求两个字符串的最大公共子串
    int findMaxCommonStr(string s1, string s2);
    // 求两个字符串的最大公共子串，考虑字符串长度
    int findMaxCommonStrInBit(string s1, string s2);

private:
    int m_nServerPort;
    int m_nDeviceNo;
    HTTPServer *m_pHTTPServer;
    std::string m_sCameraIP;

    pAIVTCb m_pAIVTCb;
    int m_nStatus;

    list<VehType_Info> m_queRegResult;
    list<SVehDel> m_queRegResultDel;

    int m_nMatchCount;

public:
    bool m_bExit;
};

#endif