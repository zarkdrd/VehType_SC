/**************************************************
     >Author: zarkdrd
     >Date: 2024-09-04 17:17:35
     >LastEditTime: 2024-09-05 15:58:13
     >LastEditors: zarkdrd
     >Description:
     >FilePath: /VehType_SC/include/convert.h
**************************************************/
/*************************************************************************
    > File Name: convert.h
    > Author: Mr Wei
    > Description:
    > Created Time: 2022-08-18
 ************************************************************************/

#ifndef _CONVERT_H_
#define _CONVERT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#if (defined(__GNUC__) && defined(CONVERT__LIBRARY))
#define CONVERT__PUBLIC __attribute__((visibility("default")))
#else
#define CONVERT__PUBLIC
#endif

#include "iconv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

     int charset_convert_UTF8_TO_GB2312(char *in_buf, size_t in_left, char *out_buf, size_t out_left);
     int charset_convert_GB2312_TO_UTF8(char *in_buf, size_t in_left, char *out_buf, size_t out_left);
     int charset_convert_UTF8_TO_UTF16(char *in_buf, size_t in_left, char *out_buf, size_t out_left);
     int charset_convert_GBK_TO_UTF8(char *in_buf, size_t in_left, char *out_buf, size_t out_left);
     int charset_convert_GBK_TO_UTF16_LE(char *in_buf, size_t in_left, char *out_buf, size_t out_left);
     int charset_convert_UTF8_TO_GBK(char *in_buf, size_t in_left, char *out_buf, size_t out_left);

     char *base64_encode(char *binData, char *base64, int binLength);
     char *base64_decode(char const *base64Str, char *debase64Str, int encodeStrLen);

     char *json_to_hex(char *json_str);
     char *hex_to_json(char *hex_str);
     char *utf8_to_gb2312(const char *utf8_str);
     char *gb2312_to_utf8(const char *gb2312_str);

     int convert_test(void);
#ifdef __cplusplus
}
#endif

#endif //_CONVERT_H_