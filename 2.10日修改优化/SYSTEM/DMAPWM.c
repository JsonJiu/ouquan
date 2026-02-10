#include "main.h"	
// 全局变量
uint16_t PWM_Breathe_Table[2000];
// 3. DMA初始化（TIM1_CH4 -> DMA2_Stream4_Channel6）
void TIM1_CH4_DMA_Init(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
    
    DMA_DeInit(DMA2_Stream4);
    while(DMA_GetCmdStatus(DMA2_Stream4) != DISABLE) {}
    
    DMA_InitStructure.DMA_Channel = DMA_Channel_6;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&TIM1->CCR4);  // TIM1通道4比较寄存器
    DMA_InitStructure.DMA_Memory0BaseAddr = (u32)PWM_Breathe_Table;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStructure.DMA_BufferSize = 2000;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  // 循环模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    
    DMA_Init(DMA2_Stream4, &DMA_InitStructure);
    DMA_Cmd(DMA2_Stream4, ENABLE);
}
