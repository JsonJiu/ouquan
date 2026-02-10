#ifndef _LED_H_
#define _LED_H_
#include  "main.h"
#define LED1 		GPIOE , GPIO_Pin_13
#define LED2 		GPIOE , GPIO_Pin_14
#define LED3 		GPIOE , GPIO_Pin_15

#define LED1_ON 		GPIO_ResetBits(GPIOE , GPIO_Pin_13)
#define LED2_ON 		GPIO_ResetBits(GPIOE , GPIO_Pin_14)
#define LED3_ON 		GPIO_ResetBits(GPIOE , GPIO_Pin_15)



#define LED1_OFF 		GPIO_SetBits(GPIOE , GPIO_Pin_13)
#define LED2_OFF 		GPIO_SetBits(GPIOE , GPIO_Pin_14)
#define LED3_OFF 		GPIO_SetBits(GPIOE , GPIO_Pin_15)

void LED_GPIO_Config(void);
void LED_FLAG_Run(void);
void LED_FLAG_LOOP(void);
extern uint8_t LED_thing_FLAG;
extern uint8_t LED_thing_time;
#endif
