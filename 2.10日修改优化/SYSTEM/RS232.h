
/***********************************************************************
文件名称：RS232.h
功    能：
编写时间：2025.11
编 写 人：WEJ
注    意：
***********************************************************************/
#ifndef _RS232_H
#define _RS232_H




#define RS232_REC_BUFF_SIZE				100
#define RS232_END_FLAG1	'#'			//RS232一桢数据结束标志1 
#define RS232_END_FLAG2	'*'			//RS232一桢数据结束标志2 


extern volatile unsigned char RS232_REC_Flag ;
extern volatile unsigned char RS232_buff[RS232_REC_BUFF_SIZE] ;//用于接收数据
extern volatile unsigned int RS232_rec_counter ;//用于RS232接收计数

void RS232_Send_Data(volatile unsigned char *send_buff,unsigned int length);
void USART1_Configuration(void);
void RS232_Printf(const char *format, ...);

#endif
