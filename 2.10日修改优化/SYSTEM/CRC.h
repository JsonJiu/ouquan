#ifndef _CRC_H
#define _CRC_H

u8 InvertUint8(u8 data);
u16 InvertUint16(u16 data);
u16 CRC16_MODBUS(u8* data, int length);
#endif
