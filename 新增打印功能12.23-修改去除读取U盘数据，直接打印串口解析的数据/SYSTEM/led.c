/***********************************************************************
文件名称：led.c
功    能：
编写时间：2025.11
编 写 人：WEJ
注    意：
***********************************************************************/
#include "main.h"
/***************  配置LED用到的I/O口 *******************/
uint8_t LED_thing_FLAG = 0;
uint8_t LED_thing_time = 0;
void LED_GPIO_Config(void)
{

	GPIO_InitTypeDef  GPIO_InitStructure;
	/* Enable the GPIO_LED Clock */
	RCC_AHB1PeriphClockCmd(  RCC_AHB1Periph_GPIOE, ENABLE); 		
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14| GPIO_Pin_15 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	/*初始化完后，关闭3个LED*/ 
	LED1_OFF;
	LED2_OFF;
	LED3_OFF;
}

void LED_FLAG_Run(void) //串口接受数据中断等 用于指示作用
{
	LED1_ON;
	LED_thing_FLAG = 1;
	//灯会亮一下
	LED_thing_time = 20;
}
void LED_FLAG_LOOP(void)
{
	if(LED_thing_FLAG==0)
		return;
	if(LED_thing_time!=0)
		return;
	LED_thing_FLAG = 0;
	LED1_OFF;
}
