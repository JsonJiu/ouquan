#ifndef _USART_H
#define _USART_H

#include <stdint.h>
/***********************************************************************
文件名称：USART.h
功    能：
编写时间：2025.11
编 写 人：WEJ
注    意：
***********************************************************************/
#define Index_88Frame          0     //第一个88的起始索引
#define Index_ControlCode       3     //控制码（1字节）
#define Index_time_idx    5     //时间数据长度起始索引
#define Index_5EFrame1          19     //第一个^起始索引
#define Index_Datappb      20    //数据区命令长度的索引
#define Index_5EFrame2          24    //第二个^起始索引
#define Index_diandao        25    //电导的索引
#define Index_CRC            31   //CRC数据标识索引


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
void shujuneiruDATA_Analyze(unsigned char *buf);
void LastDATA_Analyze(unsigned char *buf);
void Usart6_Init(void);
void USART6_IRQHandler(void);
unsigned short UartProtocol_DataAnalyze(unsigned char *buf);
#endif
