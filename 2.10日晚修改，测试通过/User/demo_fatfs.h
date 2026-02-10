#ifndef _DEMO_SDIO_FATFS_H
#define _DEMO_SDIO_FATFS_H

/* 供外部调用的函数声明 */
void DemoFatFS(void);
void CreateNewFile(unsigned char *dataBuf, uint16_t dataLen);
void CreateNewCSVFile(unsigned char *dataBuf, uint16_t dataLen);
void SetCSVFileReadOnly(void);
void ReadlastlineData(void);
void USB_PrepareFileSystem(void);
#endif


