
#include "bsp.h"				/* 底层硬件驱动 */
#include "demo_fatfs.h"
#include "main.h"	
#include <stdio.h>
#include <string.h>
#include "HONG.h"
#include "USART.h"
#include "DMA.h"
#include "delay.h"
#include "RS232.h"
extern Data_t Data;  // 全局变量
void SystemClock_Info(void)
{
    RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);
    
    printf("系统时钟频率: %lu Hz\n", RCC_Clocks.SYSCLK_Frequency);
    printf("HCLK频率: %lu Hz\n", RCC_Clocks.HCLK_Frequency);
    printf("PCLK1频率: %lu Hz\n", RCC_Clocks.PCLK1_Frequency);
    printf("PCLK2频率: %lu Hz\n", RCC_Clocks.PCLK2_Frequency);
    
    // TIM2挂在APB1上，如果APB1分频系数不为1，定时器时钟会倍频
    uint32_t TIM2_Clock = RCC_Clocks.PCLK1_Frequency;
    if (RCC_Clocks.PCLK1_Frequency != RCC_Clocks.HCLK_Frequency) {
        TIM2_Clock = RCC_Clocks.PCLK1_Frequency * 2;
    }
    printf("TIM2时钟频率: %lu Hz\n", TIM2_Clock);
}
int main(void)
{
	/*
		ST固件库中的启动文件已经执行了 SystemInit() 函数，该函数在 system_stm32f4xx.c 文件，主要功能是
	配置CPU系统的时钟，内部Flash访问时序，配置FSMC用于外部SRAM
	*/

			/* 硬件初始化 */
	LED_GPIO_Config(); //LED 端口初始化 
	TIM_Configuration();
	USART6_DMA_Tx_Init();
	USART6_DMA_Rx_Init(); 
	Usart6_Init();
	USART1_Configuration();
  DMA_PWM_Init();
	DMA_Send(RX_Buffer,ch_num);
	rx_flag=0; 
  ch_num = 0;	
	DemoFatFS();	/* FatFS文件系统演示程序 */
}

