/***********************************************************************
文件名称：timer.c
功    能：
编写时间：2025.12
编 写 人：WEJ
注    意：
***********************************************************************/
#include  "main.h"
__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */
void GPIO_ToggleBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
uint8_t p=0;
//定时器3配置
//定时器溢出时间计算方法:Tout=((4999+1)*(8399+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
 //使用定时器3，定时器2已被U盘所使用，这样会导致冲突
void TIM_Configuration(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  ///使能TIM2时钟
	
	TIM_TimeBaseInitStructure.TIM_Period = 999; 	//自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler = 83;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);//初始化TIM3
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //允许定时器3更新中断
	TIM_Cmd(TIM3,ENABLE); //使能定时器3
	
		
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* Enable CANx RX0 interrupt IRQ channel */
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}



////定时器3中断函数
//每隔	time（ms）中断一次
void TIM3_IRQHandler(void)
{
	
// 	CLI();			//关闭总中断
	if(TIM_GetITStatus(TIM3,TIM_IT_Update) != RESET) 
	{
		TIM_ClearITPendingBit(TIM3,TIM_FLAG_Update);
		if(LED_thing_time>0)
		{
		LED_thing_time--;
		}
		LocalTime+=10;//10ms增量

		
	}
}
