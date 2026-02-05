#include "stm32f4xx.h"
#include <stdio.h>
#include <string.h>
#include "HONG.h"
#include "DMA.h"
#include "USART.h"
#include "demo_fatfs.h"
#include "CRC.h"
#include "RS232.h"
#include <stdlib.h>
#include <string.h>
#include "led.h"
/***********************************************************************
文件名称：USART.c
功    能：
更新时间：2026.2.4
版本：V4.0
编 写 人：WEJ
注    意：本次增加CSV文件创建，并修改文件名带日期，现在程序同时支持txt文件和csv文件的写入，2.4日更改写入的格式。
***********************************************************************/
extern uint16_t QDvalue;   // txt文件写入启动标志	
extern uint32_t writeCount;     // txt文件写入次数
extern uint32_t csvWriteCount;  // CSV文件写入次数
extern uint16_t crc_value;      // CRC校验值
extern uint16_t value;      	// 接收的CRC值
extern uint8_t *ptr;     	// 指向接收数据的指针
extern uint8_t s_flag;
uint16_t DYvalue=0;      	// 打印启动标志
uint16_t CSVvalue=0;        // CSV文件写入启动标志 	
#define PRINTF_BUF_SIZE 256
uint8_t printf_buf[PRINTF_BUF_SIZE];
uint16_t buf_index = 0;
Data_t Data; // 全局变量
uint8_t i = 0;
// #define MAX_LENGTH     (255)

void Usart6_Init(void) // 1.初始化串口
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 中断配置
	NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // 抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; // 子优先级
	NVIC_Init(&NVIC_InitStructure);

	USART_DeInit(USART6);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);  // 使能GPIOC时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE); // 使能USART6时钟

	// 串口6对应引脚复用映射
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6); // GPIOC6复用为USART6
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6); // GPIOC7复用为USART6

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;		  // GPIOC6与GPIOC7
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	  // 复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	  // 推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	  // 上拉
	GPIO_Init(GPIOC, &GPIO_InitStructure);			  // 初始化PC6，PC7

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;	 // GPIOC7
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; // 复用功能
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/*  配置串口硬件参数 */
	USART_InitStructure.USART_BaudRate = 9600;										// 波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// 字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// 一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								// 无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// 收发模式
	USART_Init(USART6, &USART_InitStructure);										// 初始化串口6

	USART_Cmd(USART6, ENABLE);

	USART_ClearFlag(USART6, USART_FLAG_TC);
	//	   while (USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET);	//等待空闲帧发送完成后再清零发送完成标志（警告：如果不使能USART_Mode_Tx，会导致单片机在这里死机）
	//	  USART_ClearFlag(USART6, USART_FLAG_TC);	//清除发送完成标志
	//	    USART_ClearFlag(USART6, USART_FLAG_RXNE);
	//      USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);                    // 使能接收中断
	// 用于判断是否接收完一包数据..

	USART_ITConfig(USART6, USART_IT_IDLE, ENABLE);	// 开启USART1空闲中断
	USART_ITConfig(USART6, USART_IT_RXNE, DISABLE); // 禁止USART1接收不为空中断
	USART_ITConfig(USART6, USART_IT_TXE, DISABLE);	// 禁止USART1发送空中断
	USART_ITConfig(USART6, USART_IT_TC, DISABLE);	// 禁止USART1传输完成中断

	USART_DMACmd(USART6, USART_DMAReq_Tx, ENABLE); // 使能串口1的DMA发送
	USART_DMACmd(USART6, USART_DMAReq_Rx, ENABLE); // 使能串口1的DMA接收
}

unsigned char rx_flag = 0;
int16_t ch_num;
int16_t num;
void USART6_IRQHandler(void)
{
	if (USART_GetITStatus(USART6, USART_IT_IDLE) != RESET) // 检查是否是空闲中断
	{
		USART_ClearITPendingBit(USART6, USART_IT_IDLE);
		ch_num = USART_ReceiveData(USART6); // 必须先清除总线空闲中断标识，然后读一下数据寄存器，DMA接收才会正确（先读SR，然后读DR才能清除空闲中断标识）注意：这句必须要，否则不能够清除中断标志位。

		DMA_Cmd(DMA2_Stream1, DISABLE);																					  // 关闭DMA,防止处理其间有数据
		DMA_ClearFlag(DMA2_Stream1, DMA_FLAG_TCIF1 | DMA_FLAG_FEIF1 | DMA_FLAG_DMEIF1 | DMA_FLAG_TEIF1 | DMA_FLAG_HTIF1); // 清零标志位
		ch_num = USART6_DMA_RX_BUFFER_MAX_LENGTH - DMA_GetCurrDataCounter(DMA2_Stream1);								  // 获取已经接收到的字节数

		if (ch_num > 0)
		{
			/*可以在此处处理接收到的数据*/
			rx_flag = 1;
		}
		DMA_SetCurrDataCounter(DMA2_Stream1, USART6_DMA_RX_BUFFER_MAX_LENGTH);
		DMA_Cmd(DMA2_Stream1, ENABLE);
	}
	else if (USART_GetITStatus(USART6, USART_IT_TC) != RESET) // 检查是否是发送中断
	{
		USART_ClearITPendingBit(USART6, USART_IT_TC);

		DMA_ClearFlag(DMA2_Stream6, DMA_FLAG_TCIF6 | DMA_FLAG_FEIF6 | DMA_FLAG_DMEIF6 | DMA_FLAG_TEIF6 | DMA_FLAG_HTIF6);
		DMA_SetCurrDataCounter(DMA2_Stream6, 0); // 清除数据长度
	}
}

int fputc(int ch, FILE *f)
{
	USART_ClearFlag(USART1, USART_FLAG_TC);
	USART_SendData(USART6, (unsigned char)ch);
	while (USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET)
		;
	return ch;
}
int fputc1(int ch, FILE *f)
{
	// 存储字符到缓冲区
	printf_buf[buf_index++] = (uint8_t)ch;
	//         ^^^^^^^^^^     ^^^^^^^^^^
	//         先存后加1      类型转换

	if (ch == '\n') // 遇到换行就发送
	{
		DMA_Send(printf_buf, buf_index);
		buf_index = 0; // 重置索引
	}
	return ch;
}
/**
 * 串口协议解析函数
 * @param buf 输入数据缓冲区
 * @param buf_temp 临时缓冲区
 * @return 返回解析状态码
 */
unsigned short UartProtocol_DataAnalyze(unsigned char *buf)
{
	unsigned short ctlcode;
	unsigned short crc_value;
	uint16_t pHead = 0;
	uint16_t data_len = strlen((char *)buf); // 检验长度
	while (1)
	{
		if (pHead + 4 > data_len)
		{
			break;
		}
		// 检查帧头是否为0x88
		if (buf[pHead] == 0x88 && buf[pHead + 1] == 0x67 && buf[pHead + 2] == 0x96)
		{
			// 获取控制码
			ctlcode = buf[pHead + 3];
			ctlcode = (ctlcode << 8) | buf[pHead + 4]; // 低字节

			// 根据控制码进行不同的处理
			switch (ctlcode)
			{
			case 0x9BB7: // txt文件数据导出启动
				writeCount = 0;
				QDvalue = 1;
				printf("page14.t1.txt=\"启用txt下载\"\xff\xff\xff");
				pHead += 4;				
				break;
			case 0x9BB8: // csv文件数据导出启动
				csvWriteCount = 0;
				CSVvalue = 1;
				printf("page14.t1.txt=\"启用CSV下载\"\xff\xff\xff");
				pHead += 4;				
				break;
			case 0x5BB4: // 数据导出停止
		   		if(QDvalue == 1)
		   		{
					QDvalue = 0;
					printf("page14.t1.txt=\"停止txt下载\"\xff\xff\xff");
		   		}
		   		if(CSVvalue == 1)
		   		{
					CSVvalue = 0;
					SetCSVFileReadOnly();  /* 停止时设置CSV文件为只读 */
					printf("page14.t1.txt=\"停止CSV下载\"\xff\xff\xff");
		   		}
				pHead += 4;
				break;
			case 0x3BB3: // 启动打印
				DYvalue = 1;
				pHead += 4;
				printf("page14.t1.txt=\"启动打印\"\xff\xff\xff");
				break;
			case 0x3BB4: // 停止打印
				DYvalue = 0;
				pHead += 4;
				printf("page14.t1.txt=\"停止打印\"\xff\xff\xff");
				break;
			case 0x4BB1:								   // 文本内容最后一行数据写入U盘
				crc_value = CRC16_MODBUS(&buf[pHead], 39); // 计算发来数据的检验码
				ptr = &buf[pHead] + 39;					   // 指向数组的倒数第二个元素
				value = *((uint16_t *)ptr);				   // 读取发来的校验码
				if (value == crc_value)
				{
					LED_FLAG_Run();
					printf("page14.t1.txt=\"检验正确\"\xff\xff\xff");
					if (QDvalue == 1)
					{						
						CreateNewFile(); /* 创建TXT文件,写入数据 */
					}
					if (CSVvalue == 1)
					{
						CreateNewCSVFile(); /* 创建CSV文件,写入数据 */
					}
				}
				else
				{
					printf("page14.t1.txt=\"检验错误\"\xff\xff\xff");
				}
				pHead += 40;
				break;	
			case 0x4BB2:								   // 数据控件数据写入U盘
				crc_value = CRC16_MODBUS(&buf[pHead], 31); // 计算发来数据的检验码
				ptr = &buf[pHead] + 31;					   // 指向数组的倒数第二个元素
				value = *((uint16_t *)ptr);				   // 读取发来的校验码
				if (value == crc_value)
				{
					LED_FLAG_Run();
					printf("page14.t1.txt=\"检验正确\"\xff\xff\xff");
					if(DYvalue == 1 )
					{
					i++;
					shujuneiruDATA_Analyze(&buf[pHead]); // 数据解析打印
					}					
				}
				else
				{
					printf("page14.t1.txt=\"检验错误\"\xff\xff\xff");
				}
				pHead += 32;
				break;

			default:
				break;
			}
		}
		pHead++;
	}
}

// 处理小数
float parse_ascii_float(const unsigned char *str, uint8_t len)
{
	char temp[16] = {0};

	if (len > 15)
		len = 15; // 防止溢出
	memcpy(temp, str, len);
	temp[len] = '\0'; // 添加字符串结束符

	return atof(temp); // 自动处理小数点
}
// 辅助函数：解析2位ASCII数字
static uint8_t parse_2digit(const unsigned char *str)
{
	return (str[0] - '0') * 10 + (str[1] - '0');
}

// 辅助函数：解析4位ASCII数字
static uint16_t parse_4digit(const unsigned char *str)
{
	return (str[0] - '0') * 1000 +
		   (str[1] - '0') * 100 +
		   (str[2] - '0') * 10 +
		   (str[3] - '0');
}
void shujuneiruDATA_Analyze(unsigned char *buf) // 接收数组解析
{
	if (buf[Index_5EFrame1] == 0x5E && buf[Index_5EFrame2] == 0x5E) // 检验分隔符"^"
	{

		Data.year = parse_4digit(&buf[Index_time_idx]);		   // 解析年月日
		Data.month = parse_2digit(&buf[Index_time_idx + 4]);   //
		Data.day = parse_2digit(&buf[Index_time_idx + 6]);	   //
		Data.hour = parse_2digit(&buf[Index_time_idx + 8]);	   //
		Data.minute = parse_2digit(&buf[Index_time_idx + 10]); //
		Data.second = parse_2digit(&buf[Index_time_idx + 12]); //

		// ========== 解析浓度和电导率 ==========
		uint8_t ppb_len = Index_5EFrame2 - Index_Datappb;
		Data.concentration_ppb = parse_ascii_float(&buf[Index_Datappb], ppb_len);

		uint8_t conductivity_len = Index_CRC - Index_diandao;
		Data.conductivity = parse_ascii_float(&buf[Index_diandao], conductivity_len);

		// 发送到打印机
		RS232_Printf("\r\n--------------------------------\r\n");
		RS232_Printf("时间: %04d-%02d-%02d %02d:%02d:%02d\n浓度: %.2f ppb\n电导率: %.4f us/cm\r\n",
					 Data.year, Data.month, Data.day,
					 Data.hour, Data.minute, Data.second,
					 Data.concentration_ppb, Data.conductivity);
		if (i == 1)
		{
			printf("page14.t2.txt=\"首次打印成功\"\xff\xff\xff");
		}
		else
		{
			printf("page14.t2.txt=\"打印成功\"\xff\xff\xff");
		}
	}
}
void LastDATA_Analyze(unsigned char *buf) // 文本内容最后一行解析
{
	int data_len = strlen((char *)buf); // 检验长度
										// 可在这里修改长度值
	if (data_len == 26)
	{
		uint8_t len = 0;
		Data.year = parse_4digit(&buf[len]);		// 解析年月日
		Data.month = parse_2digit(&buf[len + 4]);	//
		Data.day = parse_2digit(&buf[len + 6]);		//
		Data.hour = parse_2digit(&buf[len + 8]);	//
		Data.minute = parse_2digit(&buf[len + 10]); //
		Data.second = parse_2digit(&buf[len + 12]); //

		// ========== 解析浓度和电导率 ==========
		Data.concentration_ppb = parse_ascii_float(&buf[len + 15], 4);
		Data.conductivity = parse_ascii_float(&buf[len + 20], 6);

		// 发送到打印机
		RS232_Printf("\r\n--------------------------------\r\n");
		RS232_Printf("时间: %04d-%02d-%02d %02d:%02d:%02d\n浓度: %.2f ppb\n电导率: %.4f us/cm\r\n",
					 Data.year, Data.month, Data.day,
					 Data.hour, Data.minute, Data.second,
					 Data.concentration_ppb, Data.conductivity);
		if (i == 1)
		{
			printf("page14.t2.txt=\"首次打印成功\"\xff\xff\xff");
		}
		else
		{
			printf("page14.t2.txt=\"打印成功\"\xff\xff\xff");
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: CSVData_Analyze
*	功能说明: 解析接收数据并转换为CSV格式字符串
*	形    参：buf - 输入数据缓冲区，csvOut - 输出的CSV格式字符串
*	返 回 值: CSV字符串长度，0表示失败
*   输出格式: "01/27/2026 10:35:17,6.51,1.9607"
*********************************************************************************************************
*/
uint16_t CSVData_Analyze(unsigned char *buf, char *csvOut)
{
	uint16_t len = 0;
	
	// 检验分隔符"^" (CSV新格式有4个分隔符)
	if (buf[CSV_Index_5E1] == 0x5E && buf[CSV_Index_5E2] == 0x5E && 
	    buf[CSV_Index_5E3] == 0x5E && buf[CSV_Index_5E4] == 0x5E)
	{
		// 解析日期 "2026.02.02" 格式
		uint16_t year = parse_4digit(&buf[CSV_Index_date]);      // 索引5-8: "2026"
		uint8_t month = parse_2digit(&buf[CSV_Index_date + 5]);  // 索引10-11: "02" (跳过".")
		uint8_t day = parse_2digit(&buf[CSV_Index_date + 8]);    // 索引13-14: "02" (跳过".")
		
		// 解析时间 "14:39:14" 格式
		uint8_t hour = parse_2digit(&buf[CSV_Index_time]);       // 索引16-17: "14"
		uint8_t minute = parse_2digit(&buf[CSV_Index_time + 3]); // 索引19-20: "39" (跳过":")
		uint8_t second = parse_2digit(&buf[CSV_Index_time + 6]); // 索引22-23: "14" (跳过":")
		
		// 解析浓度 "0.00"
		uint8_t ppb_len = CSV_Index_5E3 - CSV_Index_ppb;  // 29 - 25 = 4字节
		char ppb_str[16] = {0};
		memcpy(ppb_str, &buf[CSV_Index_ppb], ppb_len);
		
		// 解析电导率 "0.0000"
		uint8_t conductivity_len = CSV_Index_5E4 - CSV_Index_diandao;  // 36 - 30 = 6字节
		char conductivity_str[16] = {0};
		memcpy(conductivity_str, &buf[CSV_Index_diandao], conductivity_len);
		
		 // 解析序号 "15"
		// char temp_str[8] = {0};
		// uint8_t temp_len = CSV_Index_CRC - CSV_Index_temp;  // 39 - 37 = 2字节
		// memcpy(temp_str, &buf[CSV_Index_temp], temp_len);
		
		// 格式化为CSV行: "MM/DD/YYYY HH:MM:SS,浓度,电导率,序号"
		len = sprintf(csvOut, "%02d/%02d/%04d %02d:%02d:%02d,%s,%s",
					  month, day, year, hour, minute, second,
					  ppb_str, conductivity_str);
	}
	
	return len;
}
