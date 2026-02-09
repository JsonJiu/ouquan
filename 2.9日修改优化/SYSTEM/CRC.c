#include "stm32f4xx.h"
#include <stdio.h>
#include <string.h>


u8 InvertUint8(u8 data)
{
   int i;
   u8 newtemp8 = 0;
   for (i = 0; i < 8; i++)
   {
      if ( (data & (1 << i) ) != 0) newtemp8 |= (u8)(1 << (7 - i));
   }
   return newtemp8;
}

u16 InvertUint16(u16 data)
{
   int i;
   u16 newtemp16 = 0;
   for (i = 0; i < 16; i++)
   {
      if ( (data & (1 << i) ) != 0) newtemp16 |= (u16)(1 << (15 - i));
   }
   return newtemp16;
}
u16 CRC16_MODBUS(u8* data, int length) {
    u16 wCRCin = 0xFFFF;
    u16 wCPoly = 0x8005;
	  int j;
    while (length--) { // 使用length--代替lenth--
        u8 wChar = InvertUint8(*data++); // 先取反再更新指针
        wCRCin ^= wChar << 8;
   			
        for(j = 8; j != 0; j--) { // 从高位到低位处理
            if ((wCRCin & 0x8000) != 0) {
                wCRCin = (u16)((wCRCin << 1) ^ wCPoly);
            } else {
                wCRCin <<= 1;
            }
        }
    }
    return InvertUint16(wCRCin); // 最后取反得到CRC校验码
}
