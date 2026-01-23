#include "main.h"	
#include "DMAPWM.h"
extern uint16_t PWM_Breathe_Table[2000];

// 1. 初始化PWM波形表
void PWM_Table_Init(void)
{
    u16 i;
    for(i = 0; i < 1000; i++)
    {
        PWM_Breathe_Table[i] = i;  // 0->500 变亮
    }
    for(i = 0; i < 1000; i++)
    {
        PWM_Breathe_Table[1000 + i] = 1000 - i;  // 500->0 变暗
    }
}
void TIM1_PWM_Init(void)
{		 					 
	
	
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE); //TIM1时钟使能 	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); 	//使能PORTE时钟		
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;           
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//速度100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        //上拉
	GPIO_Init(GPIOE,&GPIO_InitStructure);              //初始化PE14
	
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource14,GPIO_AF_TIM1); //GPIOPE14复用为定时器1
	  
	TIM_TimeBaseStructure.TIM_Prescaler=168-1;  //定时器分频
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseStructure.TIM_Period=500;   //自动重装载值
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;	
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseStructure);//初始化定时器1
	
	//初始化TIM1 Ch4 PWM模式	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //TIM脉冲宽度调制模式1
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//OC输出使能
  //TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;//OC互补输出使能
  TIM_OCInitStructure.TIM_Pulse = 0;//自动重装载值
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;//设置输出极性 
  //TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCPolarity_Low;//设置互补输出极性
  //TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
  //TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
	TIM_OC4Init(TIM1, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM1 OC4
	TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);  //使能TIM1 4在CCR1上的预装载寄存器
 
  TIM_ARRPreloadConfig(TIM1,ENABLE);//ARPE使能
 	TIM_DMACmd(TIM1, TIM_DMA_CC4, ENABLE);
	TIM_Cmd(TIM1, ENABLE);  //使能TIM1
  TIM_CtrlPWMOutputs(TIM1, ENABLE);	//PWM输出使能，TIM1和TIM8一定要打开
										  
}  
void DMA_PWM_Init(void)
{

    PWM_Table_Init();      // 生成呼吸灯波形表
    TIM1_PWM_Init();       // 初始化PWM
    TIM1_CH4_DMA_Init();   // 初始化DMA

}
