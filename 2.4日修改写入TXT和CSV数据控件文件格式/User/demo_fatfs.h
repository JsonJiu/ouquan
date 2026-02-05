#ifndef _DEMO_SDIO_FATFS_H
#define _DEMO_SDIO_FATFS_H

/* 供外部调用的函数声明 */
void DemoFatFS(void);
void CreateNewFile(void);
void CreateNewCSVFile(void);
void SetCSVFileReadOnly(void);
void ReadlastlineData(void);
#endif


