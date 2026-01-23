#ifndef _DMA_H_
#define _DMA_H
#include "HONG.h"		/* 包含 USART6_DMA_RX_BUFFER_MAX_LENGTH 等宏定义 */
extern uint8_t RX_Buffer[USART6_DMA_RX_BUFFER_MAX_LENGTH];
extern uint8_t TX_Buffer[USART6_DMA_TX_BUFFER_MAX_LENGTH];

/* 提供给其他C文件调用的函数 */

void USART6_DMA_Tx_Init(void);
void DMA_Send(uint8_t *send_buffer , uint16_t nSendCount);
void USART6_DMA_Rx_Init(void);
#endif
