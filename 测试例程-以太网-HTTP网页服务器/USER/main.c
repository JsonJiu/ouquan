/**
  ******************************************************************************  
  * HTTP 客户端 - 连接云端服务器
  * 实验平台:   F407 开发板  
  *
  ******************************************************************************
  */
#include "stm32f4xx.h"
#include "./Bsp/led/bsp_led.h" 
#include "./Bsp/usart/bsp_debug_usart.h"
#include "./Bsp/systick/bsp_SysTick.h"
#include "lwip/tcp.h"
#include "netconf.h"
#include "LAN8742A.h"
#include "http_client.h"  // 使用HTTP客户端
#include "ota_update.h"   // OTA固件更新

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define HTTP_REQUEST_INTERVAL  5000  // HTTP请求间隔时间(ms)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern __IO uint8_t EthLinkStatus;
__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */
__IO uint32_t LastRequestTime = 0; /* 上次发送请求的时间 */
uint8_t http_connected = 0;  /* HTTP连接状态标志 */

/* Private function prototypes -----------------------------------------------*/
static void TIM3_Config(uint16_t period,uint16_t prescaler);
/* Private functions ---------------------------------------------------------*/
/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{
	/* 初始化LED */
	LED_GPIO_Config();
	
	/* 初始化调试串口，一般为串口1 */
	Debug_USART_Config();
	
	/* 初始化系统滴答定时器 */	
	SysTick_Init();
	
	TIM3_Config(999,899);//10ms定时器
	printf("\n========================================\n");
	printf("STM32F407 HTTP客户端示例\n");
	printf("========================================\n");
	
	printf("本地IP地址: %d.%d.%d.%d\n", IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);

	/* Configure ethernet (GPIOs, clocks, MAC, DMA) */
	ETH_BSP_Config();	
	printf("LAN8720A初始化成功\n");
	
	/* Initilaize the LwIP stack */
	LwIP_Init();	
	printf("LwIP协议栈初始化成功\n");
	
	/* 初始化HTTP客户端 */
	http_client_init();
	
	/* 初始化OTA更新模块 */
	ota_init();
	
	printf("\n准备连接云端HTTP服务器...\n");
	printf("请确保已修改http_client.h中的服务器IP地址\n");
	
	/* 延时一段时间，等待网络稳定 */
	Delay_ms(2000);
	
	while(1)
	{
		/* check if any packet received */
		if (ETH_CheckFrameReceived())
		{ 
			/* process received ethernet packet */
			LwIP_Pkt_Handle();
		}
		
		/* handle periodic timers for LwIP */
		LwIP_Periodic_Handle(LocalTime);
		
		/* OTA更新任务 */
		ota_task();
		
		/* HTTP客户端状态机 */
		if (!http_connected) {
			/* 尝试连接到HTTP服务器 */
			if (http_client_connect() == ERR_OK) {
				http_connected = 1;
				LastRequestTime = LocalTime;
				Delay_ms(1000); // 等待连接建立
				
				/* 连接成功后发送第一个请求 */
				if (http_client_get_state() == HTTP_CLIENT_CONNECTED) {
					printf("\n--- 发送GET请求 ---\n");
					http_client_send_get_request("/api/data");
				}
			}
		} 
		else {
			/* 已连接状态，定期发送请求 */
			if (http_client_get_state() == HTTP_CLIENT_CONNECTED) {
				/* 每隔一段时间发送一次请求 */
				if ((LocalTime - LastRequestTime) > HTTP_REQUEST_INTERVAL) {
					printf("\n--- 发送周期性请求 ---\n");
					
					/* 示例：发送POST请求，数据为JSON格式 */
					const char *json_data = "{\"sensor\":\"temperature\",\"value\":25.5}";
					http_client_send_post_request("/api/sensor", json_data);
					
					LastRequestTime = LocalTime;
					LED1_TOGGLE; // LED闪烁表示正在通讯
				}
			}
			else if (http_client_get_state() == HTTP_CLIENT_ERROR || 
			         http_client_get_state() == HTTP_CLIENT_CLOSED) {
				/* 连接断开或出错，重新连接 */
				printf("\n连接断开，3秒后重新连接...\n");
				http_connected = 0;
				Delay_ms(3000);
			}
		}
		
		/* 短延时，避免CPU占用过高 */
		Delay_ms(10);
	}
}

/**
  * @brief  通用定时器3中断初始化
  * @param  period : 自动重装值。
  * @param  prescaler : 时钟预分频数
  * @retval 无
  * @note   定时器溢出时间计算方法:Tout=((period+1)*(prescaler+1))/Ft us.
  *          Ft=定时器工作频率,为SystemCoreClock/2=90,单位:Mhz
  */
static void TIM3_Config(uint16_t period,uint16_t prescaler)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  ///使能TIM3时钟
	
	TIM_TimeBaseInitStructure.TIM_Prescaler=prescaler;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_Period=period;   //自动重装载值
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //允许定时器3更新中断
	TIM_Cmd(TIM3,ENABLE); //使能定时器3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //定时器3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  定时器3中断服务函数
  * @param  无
  * @retval 无
  */
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //溢出中断
	{
		LocalTime+=10;//10ms增量
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //清除中断标志位
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
