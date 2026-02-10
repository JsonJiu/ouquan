#include "stm32f4xx.h"
#include <stdio.h>
#include <string.h>
#include "HONG.h"

//#define MAX_LENGTH		(255)

uint8_t RX_Buffer[USART6_DMA_RX_BUFFER_MAX_LENGTH];
uint8_t TX_Buffer[USART6_DMA_TX_BUFFER_MAX_LENGTH];


void USART6_DMA_Tx_Init(void)
{  
     DMA_InitTypeDef  DMA_InitStructure;


     RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);//DMA2时钟使能
     
	   DMA_DeInit(DMA2_Stream6); 
     while (DMA_GetCmdStatus(DMA2_Stream6) != DISABLE);//等待DMA2_Stream6可配置 
        

      
        /* 配置 DMA Stream */
      DMA_InitStructure.DMA_Channel = DMA_Channel_5;  //通道5
      DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART6->DR;           //DMA 对应的外设USART4->DR地址
      DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)TX_Buffer;      // 内存存储基地址

      DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;//存储器到外设模式
      DMA_InitStructure.DMA_BufferSize = USART6_DMA_TX_BUFFER_MAX_LENGTH;//数据传输量
      DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设非增量模式
      DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//存储器增量模式
      DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//外设数据长度:8位
      DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//存储器数据长度 8位
      DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;// 循环模式
      DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;//中等优先级
      DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable; //FIFO模式不开启 
      DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;    
      DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//存储器突发单次传输
      DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//外设突发单次传输
      DMA_Init(DMA2_Stream6, &DMA_InitStructure);//初始化DMA Stream

      DMA_Cmd(DMA2_Stream6, ENABLE);                  /* DMA2_Stream6 enable */ // 在发送的时候才开启DMA

}


void DMA_Send(uint8_t *send_buffer , uint16_t nSendCount)
{	
 
	if (nSendCount < USART6_DMA_TX_BUFFER_MAX_LENGTH)
	{
		memcpy(TX_Buffer , send_buffer , nSendCount);
		DMA_Cmd(DMA2_Stream6,DISABLE);                    //关闭DMA传输
		while (DMA_GetCmdStatus(DMA2_Stream6) != DISABLE);	//确保DMA可以被设置
		DMA_SetCurrDataCounter(DMA2_Stream6 , nSendCount);  //数据传输量
		DMA_Cmd(DMA2_Stream6,ENABLE);               		//开启DMA传输
	}
			    while(1)
		    {
				if(DMA_GetFlagStatus(DMA2_Stream6,DMA_FLAG_TCIF6)!=RESET)//等待DMA2_Steam6传输完成
				{ 
					DMA_ClearFlag(DMA2_Stream6,DMA_FLAG_TCIF6);//清除DMA2_Steam6传输完成标志
					break; 
			}
			}
}


void USART6_DMA_Rx_Init(void)
{  
        DMA_InitTypeDef  DMA_InitStructure;


        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);//DMA2时钟使能

        DMA_DeInit(DMA2_Stream1); 
	     while (DMA_GetCmdStatus(DMA2_Stream1) != DISABLE);//等待DMA可配置 
      

        
        /* 配置 DMA Stream */
        DMA_InitStructure.DMA_Channel = DMA_Channel_5;  //通道4
        DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART6->DR;           //DMA 对应的外设USART1->DR地址
        DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)RX_Buffer;       // 内存存储基地址

        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory ;//外设到存储器模式
        DMA_InitStructure.DMA_BufferSize = USART6_DMA_RX_BUFFER_MAX_LENGTH;//数据传输量
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设非增量模式
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//存储器增量模式
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//外设数据长度:8位
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//存储器数据长度 8位
        DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;// 循环模式
        DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;//高优先级
        DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable; //FIFO模式不开启  
        DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;    
        DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//存储器突发单次传输
        DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//外设突发单次传输
        DMA_Init(DMA2_Stream1, &DMA_InitStructure);//初始化DMA Stream

        DMA_Cmd(DMA2_Stream1, ENABLE);                  /* DMA2_Stream2 enable */

}

