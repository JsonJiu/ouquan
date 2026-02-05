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
/* ========== 0x4BB2 打印数据包索引（原格式）========== */
/* 格式: 88 67 96 4B B2 + "20260204191847" + "^" + "6.51" + "^" + "1.9607" + CRC */
#define Index_88Frame          0     //第一个88的起始索引
#define Index_ControlCode       3     //控制码（1字节）
#define Index_time_idx    5     //时间数据长度起始索引
#define Index_5EFrame1          19     //第一个^起始索引
#define Index_Datappb      20    //数据区命令长度的索引
#define Index_5EFrame2          24    //第二个^起始索引
#define Index_diandao        25    //电导的索引
#define Index_CRC            31   //CRC数据标识索引

/* ========== 0x4BB1 CSV数据包索引（新格式）========== */
/* 格式: 88 67 96 4B B1 + "2026.02.02" + "^" + "14:39:14" + "^" + "0.00" + "^" + "0.0000" + "^" + "15" + CRC */
#define CSV_Index_date         5      //日期起始索引 "2026.02.02" (10字节)
#define CSV_Index_5E1          15     //第一个^
#define CSV_Index_time         16     //时间起始索引 "14:39:14" (8字节)
#define CSV_Index_5E2          24     //第二个^
#define CSV_Index_ppb          25     //浓度索引 "0.00" (4字节)
#define CSV_Index_5E3          29     //第三个^
#define CSV_Index_diandao      30     //电导率索引 "0.0000" (6字节)
#define CSV_Index_5E4          36     //第四个^
#define CSV_Index_temp         37     //序号索引  (2字节)
#define CSV_Index_CRC          39     //CRC索引


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
uint16_t CSVData_Analyze(unsigned char *buf, char *csvOut);
void Usart6_Init(void);
void USART6_IRQHandler(void);
unsigned short UartProtocol_DataAnalyze(unsigned char *buf);
#endif
