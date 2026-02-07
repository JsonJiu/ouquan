#ifndef _USART_H
#define _USART_H

#include <stdint.h>
/***********************************************************************
文件名称：USART.h
功    能：
编写时间：2026.1
编 写 人：WEJ
注    意：
***********************************************************************/
/* ========== 0x4BB2 打印数据包索引（支持不固定长度）========== */
/* 格式: 88 67 96 4B B2 + "20260204191847" + "^" + TOC + "^" + 电导率 + CRC */
/* 注意: TOC和电导率长度不固定，通过^分隔符动态解析 */


// 添加数据结构定义
typedef struct {
    // 时间数据
    uint16_t year;      // 年（表示2020-2099）
    uint8_t month;     // 月（1-12）
    uint8_t day;       // 日（1-31）
    uint8_t hour;      // 时（0-23）
    uint8_t minute;    // 分（0-59）
    uint8_t second;    // 秒（0-59）
    
    // 测量数据
    float concentration_ppb;  // 浓度（ppb）
    float conductivity;       // 电导率
    
} Data_t;

/* 提供给其他C文件调用的函数 */
extern unsigned char rx_flag ;
extern int16_t ch_num;
void shujuneiruDATA_Analyze(unsigned char *buf, uint16_t buf_len);
void LastDATA_Analyze(unsigned char *buf);
uint16_t CSVData_Analyze(unsigned char *buf, char *csvOut);
void Usart6_Init(void);
void USART6_IRQHandler(void);
unsigned short UartProtocol_DataAnalyze(unsigned char *buf);
#endif
