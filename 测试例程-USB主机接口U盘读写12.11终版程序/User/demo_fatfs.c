
#include "bsp.h"
#include "ff.h"			/* FatFS文件系统模块*/
#include "demo_fatfs.h"
#include "usbh_bsp_msc.h"
#include "HONG.h"
#include "USART.h"
#include "DMA.h"
#include "CRC.h"
#include "RS232.h"
#include "main.h"
/* 用于测试读写速度 */
#define TEST_FILE_LEN			(2*1024*1024)	/* 用于测试的文件长度 */
#define BUF_SIZE				(4*1024)		/* 每次读写SD卡的最大数据长度 */
uint8_t g_TestBuf[BUF_SIZE];

/* 仅允许本文件内调用的函数声明 */
static void DispMenu(void);
static void ViewRootDir(void);
static void ReadFileData(void);
static void CreateDir(void);
static void DeleteDirFile(void);
static void WriteFileTest(void);
static void Delay(__IO uint32_t nCount);

/*
*********************************************************************************************************
*	函 数 名: DemoFatFS
*	功能说明: FatFS文件系统演示主程序
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
uint16_t crc_value=0;
uint16_t value=0;
uint16_t QDvalue=0;
uint8_t *ptr;
uint32_t writeCount = 0;  // 写入次数
uint8_t s_flag = 0;  // 写入成功标志位
unsigned char buff[2]={0x0a,0x0d};
void DemoFatFS(void)
{
	uint8_t cmd;

	  /* Init Host Library */
#ifdef USE_USB_OTG_FS
		USBH_Init(&USB_OTG_Core,
			USB_OTG_FS_CORE_ID,
            &USB_Host,
            &USBH_MSC_cb,
            &USR_cb);
	#else
		USBH_Init(&USB_OTG_Core,
			USB_OTG_HS_CORE_ID,
            &USB_Host,
            &USBH_MSC_cb,
            &USR_cb);
	#endif

	/* 打印命令列表，用户可以通过串口操作指令 */
	//DispMenu();
	while (1)
	{
		USBH_Process(&USB_OTG_Core, &USB_Host);
    LED_FLAG_LOOP();
		if (rx_flag == 1)	/* 从串口读入一个字符(非阻塞方式) */
		{
			  UartProtocol_DataAnalyze(RX_Buffer);
				rx_flag=0;
				memset(RX_Buffer, 0, USART6_DMA_RX_BUFFER_MAX_LENGTH);			  
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: DispMenu
*	功能说明: 显示操作提示菜单
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispMenu(void)
{
	printf("\r\n------------------------------------------------\r\n");
	printf("请选择操作命令:\r\n");
	printf("1 - 显示根目录下的文件列表\r\n");
	printf("2 - 创建一个新文件STM32F407.txt\r\n");
	printf("3 - 读STM32F407.txt文件的内容\r\n");
	printf("4 - 创建目录\r\n");
	printf("5 - 删除文件和目录\r\n");
	printf("6 - 读写文件速度测试\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: ViewRootDir
*	功能说明: 显示SD卡根目录下的文件名
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void ViewRootDir(void)
{
	/* 本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用 */
	FRESULT result;
	FATFS fs;
	DIR DirInf;
	FILINFO FileInf;
	uint32_t cnt = 0;
	char lfname[256];

 	/* 挂载文件系统 */
	result = f_mount(FS_USB, &fs);	/* Mount a logical drive */
	if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%d)\r\n", result);
	}

	/* 打开根文件夹 */
	result = f_opendir(&DirInf, "/"); /* 如果不带参数，则从当前目录开始 */
	if (result != FR_OK)
	{
		printf("打开根目录失败 (%d)\r\n", result);
		return;
	}

	/* 读取当前文件夹下的文件和目录 */
	FileInf.lfname = lfname;
	FileInf.lfsize = 256;

	printf("属性        |  文件大小 | 短文件名 | 长文件名\r\n");
	for (cnt = 0; ;cnt++)
	{
		result = f_readdir(&DirInf,&FileInf); 		/* 读取目录项，索引会自动下移 */
		if (result != FR_OK || FileInf.fname[0] == 0)
		{
			break;
		}

		if (FileInf.fname[0] == '.')
		{
			continue;
		}

		/* 判断是文件还是子目录 */
		if (FileInf.fattrib & AM_DIR)
		{
			printf("(0x%02d)目录  ", FileInf.fattrib);
		}
		else
		{
			printf("(0x%02d)文件  ", FileInf.fattrib);
		}

		/* 打印文件大小, 最大4G */
		printf(" %10d", FileInf.fsize);

		printf("  %s |", FileInf.fname);	/* 短文件名 */

		printf("  %s\r\n", (char *)FileInf.lfname);	/* 长文件名 */
	}

	/* 卸载文件系统 */
	f_mount(FS_USB, NULL);
}

/*
*********************************************************************************************************
*	函 数 名: CreateNewFile
*	功能说明: 在SD卡创建一个新文件，文件内容
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static uint8_t fileNumber = 1;  // 当前文件编号

static char fileName[15]; //存取文件编号的数组
//读取当前的文本序号函数ReadFileNumber
static uint8_t ReadFileNumber(void) {
    FRESULT res;
    FATFS fs;
    DIR dir; // 注意这里声明 DIR 类型的变量
    FILINFO fno;
    uint8_t maxFileNumber = 0;

    /* 尝试挂载存储设备 */
    res = f_mount(FS_USB, &fs);  // 假设 "0:" 是您的 USB 驱动器号
    if (res != FR_OK) {
        printf("挂载存储设备失败 (%d)\n", res);
        return 0;
    }

    /* 打开根目录 */
    res = f_opendir(&dir, "/");  // 打开根目录
    if (res == FR_OK) {
        /* 循环遍历目录中的所有文件 */
        while ((res = f_readdir(&dir, &fno)) == FR_OK && fno.fname[0] != 0) { // f_readdir 返回 FR_OK 且文件名不为空时继续
            if (fno.fattrib & AM_DIR) continue;  // 忽略目录

            // 检查文件扩展名是否为 .txt
            const char *ext = strrchr(fno.fname, '.');
            if (ext && strcmp(ext, ".txt") == 0) {
                // 从文件名中提取数字
                int num;
                if (sscanf(fno.fname, "%d.txt", &num) == 1 && num > maxFileNumber) {
                    maxFileNumber = (uint8_t)num;  // 找到更高的文件编号
                }
            }
        }
    } else {
        printf("打开目录失败 (%d)\n", res);
    }

    /* 卸载存储设备 */
    f_mount(FS_USB, NULL);

    return maxFileNumber;  // 返回最高的文件编号
}
void CreateNewFile(void)
{
	/* 本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用 */
	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;
	uint32_t bw;
	char buf[128];
  //char fileName[15];
	s_flag=0;
	
 if (writeCount == 0&&QDvalue==1) {
        /* 调用 ReadFileNumber 函数以获取当前最高的文件编号 */
        fileNumber = ReadFileNumber();
        if (fileNumber > 0) {
            /* 如果检测到文件编号，从下一个编号开始创建文件 */
            fileNumber++;
        } else {
            /* 如果没有找到任何文件，从1开始编号 */
            fileNumber = 1;
        }
    }
	sprintf(fileName, "%d.txt", fileNumber);
 	/* 挂载文件系统 */
	result = f_mount(FS_USB, &fs);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%d)\r\n", result);
		return;
	}

	/* 打开根文件夹 */
	result = f_opendir(&DirInf, "/"); /* 如果不带参数，则从当前目录开始 */
	if (result != FR_OK)
	{
		printf("打开根目录失败 (%d)\r\n", result);
		f_mount(FS_USB, NULL); // 卸载文件系统
		return;
	}

	/* 打开文件 */
	result = f_open(&file, fileName, FA_OPEN_ALWAYS | FA_WRITE);
	if (result == FR_OK&&QDvalue==1)
	{
	 result = f_lseek(&file, f_size(&file));
		if (ch_num > 7) {  // 确保数据长度足够（头5字节 + 尾2字节 = 至少7字节）
    result = f_write(&file, RX_Buffer + 5, ch_num - 7, &bw);//写入数据内容
}
	 //result = f_write(&file, RX_Buffer, ch_num-2, &bw);
if (result == FR_OK) {	
		/* 写入换行符 */
   const char newline[] = "\r\n";
    result = f_write(&file, newline, sizeof(newline) - 1, &bw);
	if (result == FR_OK) {
		printf("page17.t0.txt=\"%d.txt文件写入成功\"\xff\xff\xff",fileNumber);
		s_flag=1;
		//printf("%d.txt 文件写入成功\r\n",fileNumber);		
		}
}
    		
		/* 更新写入次数 */
		writeCount++;
		if (writeCount >= 300) {
        fileNumber++;  // 增加文件编号
        writeCount = 0;  // 重置写入次数
    }
	}
	else
	{
		printf("%d.txt 文件写入失败\r\n",fileNumber);
	}


	/* 关闭文件*/
	f_close(&file);
	/* 卸载文件系统 */
	f_mount(FS_USB, NULL);
	
	/* 读取一串数据 */ 
	result = f_mount(FS_USB, &fs);
	result = f_opendir(&DirInf, "/");
	result = f_open(&file,fileName, FA_OPEN_EXISTING | FA_READ); 

	f_close(&file);
	f_mount(FS_USB, NULL);

}
static void Delay(__IO uint32_t nCount)
{
  for(; nCount != 0; nCount--) ;
}
/*
*********************************************************************************************************
*	函 数 名: ReadFileData
*	功能说明: 读取文件armfly.txt前128个字符，并打印到串口
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void ReadFileData(void)
{
	/* 本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用 */
	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;
	uint32_t bw;
	char buf[128];

 	/* 挂载文件系统 */
	result = f_mount(FS_USB, &fs);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		printf("挂载文件系统失败(%d)\r\n", result);
	}

	/* 打开根文件夹 */
	result = f_opendir(&DirInf, "/"); /* 如果不带参数，则从当前目录开始 */
	if (result != FR_OK)
	{
		printf("打开根目录失败(%d)\r\n", result);
		return;
	}

	/* 打开文件 */
	result = f_open(&file,fileName, FA_OPEN_EXISTING | FA_READ);
	if (result !=  FR_OK)
	{
		printf("Don't Find File : STM32F407.txt\r\n");
		return;
	}

	/* 读取文件 */
	result = f_read(&file, &buf, sizeof(buf) - 1, &bw);
	if (bw > 0)
	{
		buf[bw] = 0;
		printf("\r\n%s.txt 文件内容 : \r\n%s\r\n",fileName,buf);
	}
	else
	{
		printf("\r\nSTM32F407.txt 文件内容 : \r\n");
	}

	/* 关闭文件*/
	f_close(&file);

	/* 卸载文件系统 */
	f_mount(FS_USB, NULL);
}

/*
*********************************************************************************************************
*	函 数 名: ReadlastData
*	功能说明: 读取文件的最后一行数据并打印到串口
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
 void ReadlastlineData(void)
{
	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;
	char buffer[256];  // 增大缓冲区以容纳可能较长的一行
	uint32_t fileSize,readSize,bytesRead;
	unsigned char *lastNewline,*lastline;
	
	/* 挂载文件系统 */
	result = f_mount(FS_USB, &fs);
	if (result != FR_OK)
	{
		printf("挂载文件系统失败(%d)\r\n", result);
		return;
	}
	
	/* 打开根文件夹 */
	result = f_opendir(&DirInf, "/");
	if (result != FR_OK)
	{
		printf("打开根目录失败(%d)\r\n", result);
		f_mount(FS_USB, NULL);
		return;
	}
	
	/* 打开文件 */
	result = f_open(&file, fileName, FA_OPEN_EXISTING | FA_READ);
	if (result != FR_OK)
	{
		printf("打开文件失败: %s (%d)\r\n", fileName, result);
		f_mount(FS_USB, NULL);
		return;
	}
	
	/* 获取文件大小 */
	fileSize = f_size(&file);
	 readSize = (fileSize < sizeof(buffer)) ? fileSize : sizeof(buffer);
		if (fileSize > sizeof(buffer)) {
        f_lseek(&file, fileSize - readSize);
    }
	    f_read(&file, buffer, readSize, &bytesRead);
			buffer[bytesRead] = '\0';
		
		lastNewline = (unsigned char*)buffer;//处理多个连续的换行符
    for (unsigned char *p = (unsigned char*) buffer + bytesRead - 1; p >=(unsigned char*) buffer; p--) {
        if (*p == '\n') {
            // 跳过末尾的空行
            if (p == (unsigned char*)buffer + bytesRead - 1) continue;
            lastNewline = p + 1; //找出倒数第一个换行符
            break;
        }
    }
    
    lastline = lastNewline;
    
    // 移除行末的 \r\n
     unsigned char *end = lastline + strlen((char*)lastline) - 1;
    while (end >= lastline && (*end == '\r' || *end == '\n')) {
        *end-- = '\0';
    }
		
		//RS232_Printf("最后一行: %s\r\n", lastline);
		LastDATA_Analyze(lastline);
		// PrintData(buf);  // 你的打印函数
	
	/* 关闭文件 */
	f_close(&file);
	
	/* 卸载文件系统 */
	f_mount(FS_USB, NULL);
}
/*
*********************************************************************************************************
*	函 数 名: CreateDir
*	功能说明: 在SD卡根目录创建Dir1和Dir2目录，在Dir1目录下创建子目录Dir1_1
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void CreateDir(void)
{
	/* 本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用 */
	FRESULT result;
	FATFS fs;

 	/* 挂载文件系统 */
	result = f_mount(FS_USB, &fs);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%d)\r\n", result);
	}

	/* 创建目录/Dir1 */
	result = f_mkdir("/Dir1");
	if (result == FR_OK)
	{
		printf("f_mkdir Dir1 Ok\r\n");
	}
	else if (result == FR_EXIST)
	{
		printf("Dir1 目录已经存在(%d)\r\n", result);
	}
	else
	{
		printf("f_mkdir Dir1 失败 (%d)\r\n", result);
		return;
	}

	/* 创建目录/Dir2 */
	result = f_mkdir("/Dir2");
	if (result == FR_OK)
	{
		printf("f_mkdir Dir2 Ok\r\n");
	}
	else if (result == FR_EXIST)
	{
		printf("Dir2 目录已经存在(%d)\r\n", result);
	}
	else
	{
		printf("f_mkdir Dir2 失败 (%d)\r\n", result);
		return;
	}

	/* 创建子目录 /Dir1/Dir1_1	   注意：创建子目录Dir1_1时，必须先创建好Dir1 */
	result = f_mkdir("/Dir1/Dir1_1"); /* */
	if (result == FR_OK)
	{
		printf("f_mkdir Dir1_1 成功\r\n");
	}
	else if (result == FR_EXIST)
	{
		printf("Dir1_1 目录已经存在 (%d)\r\n", result);
	}
	else
	{
		printf("f_mkdir Dir1_1 失败 (%d)\r\n", result);
		return;
	}

	/* 卸载文件系统 */
	f_mount(FS_USB, NULL);
}

/*
*********************************************************************************************************
*	函 数 名: DeleteDirFile
*	功能说明: 删除SD卡根目录下的 armfly.txt 文件和 Dir1，Dir2 目录
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DeleteDirFile(void)
{
	/* 本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用 */
	FRESULT result;
	FATFS fs;
	char FileName[13];
	uint8_t i;

 	/* 挂载文件系统 */
	result = f_mount(FS_USB, &fs);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%d)\r\n", result);
	}

	#if 0
	/* 打开根文件夹 */
	result = f_opendir(&DirInf, "/"); /* 如果不带参数，则从当前目录开始 */
	if (result != FR_OK)
	{
		printf("打开根目录失败(%d)\r\n", result);
		return;
	}
	#endif

	/* 删除目录/Dir1 【因为还存在目录非空（存在子目录)，所以这次删除会失败】*/
	result = f_unlink("/Dir1");
	if (result == FR_OK)
	{
		printf("删除目录Dir1成功\r\n");
	}
	else if (result == FR_NO_FILE)
	{
		printf("没有发现文件或目录 :%s\r\n", "/Dir1");
	}
	else
	{
		printf("删除Dir1失败(错误代码 = %d) 文件只读或目录非空\r\n", result);
	}

	/* 先删除目录/Dir1/Dir1_1 */
	result = f_unlink("/Dir1/Dir1_1");
	if (result == FR_OK)
	{
		printf("删除子目录/Dir1/Dir1_1成功\r\n");
	}
	else if ((result == FR_NO_FILE) || (result == FR_NO_PATH))
	{
		printf("没有发现文件或目录 :%s\r\n", "/Dir1/Dir1_1");
	}
	else
	{
		printf("删除子目录/Dir1/Dir1_1失败(错误代码 = %d) 文件只读或目录非空\r\n", result);
	}

	/* 先删除目录/Dir1 */
	result = f_unlink("/Dir1");
	if (result == FR_OK)
	{
		printf("删除目录Dir1成功\r\n");
	}
	else if (result == FR_NO_FILE)
	{
		printf("没有发现文件或目录 :%s\r\n", "/Dir1");
	}
	else
	{
		printf("删除Dir1失败(错误代码 = %d) 文件只读或目录非空\r\n", result);
	}

	/* 删除目录/Dir2 */
	result = f_unlink("/Dir2");
	if (result == FR_OK)
	{
		printf("删除目录 Dir2 成功\r\n");
	}
	else if (result == FR_NO_FILE)
	{
		printf("没有发现文件或目录 :%s\r\n", "/Dir2");
	}
	else
	{
		printf("删除Dir2 失败(错误代码 = %d) 文件只读或目录非空\r\n", result);
	}

	/* 删除文件 armfly.txt */
	result = f_unlink("STM32F407.txt");
	if (result == FR_OK)
	{
		printf("删除文件 STM32F407.txt 成功\r\n");
	}
	else if (result == FR_NO_FILE)
	{
		printf("没有发现文件或目录 :%s\r\n", "STM32F407.txt");
	}
	else
	{
		printf("删除STM32F407.txt失败(错误代码 = %d) 文件只读或目录非空\r\n", result);
	}

	/* 删除文件 speed1.txt */
	for (i = 0; i < 20; i++)
	{
		sprintf(FileName, "Speed%02d.txt", i);		/* 每写1次，序号递增 */
		result = f_unlink(FileName);
		if (result == FR_OK)
		{
			printf("删除文件%s成功\r\n", FileName);
		}
		else if (result == FR_NO_FILE)
		{
			printf("没有发现文件:%s\r\n", FileName);
		}
		else
		{
			printf("删除%s文件失败(错误代码 = %d) 文件只读或目录非空\r\n", FileName, result);
		}
	}

	/* 卸载文件系统 */
	f_mount(FS_USB, NULL);
}

/*
*********************************************************************************************************
*	函 数 名: WriteFileTest
*	功能说明: 测试文件读写速度
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void WriteFileTest(void)
{
	/* 本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用 */
	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;
	uint32_t bw;
	uint32_t i,k;
	uint32_t runtime1,runtime2,timelen;
	uint8_t err = 0;
	char TestFileName[13];
	static uint8_t s_ucTestSn = 0;

	for (i = 0; i < sizeof(g_TestBuf); i++)
	{
		g_TestBuf[i] = (i / 512) + '0';
	}

  	/* 挂载文件系统 */
	result = f_mount(FS_USB, &fs);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%d)\r\n", result);
	}

	/* 打开根文件夹 */
	result = f_opendir(&DirInf, "/"); /* 如果不带参数，则从当前目录开始 */
	if (result != FR_OK)
	{
		printf("打开根目录失败 (%d)\r\n", result);
		return;
	}

	/* 打开文件 */
	sprintf(TestFileName, "Speed%02d.txt", s_ucTestSn++);		/* 每写1次，序号递增 */
	result = f_open(&file, TestFileName, FA_CREATE_ALWAYS | FA_WRITE);

	/* 写一串数据 */
	printf("开始写文件%s %dKB ...\r\n", TestFileName, TEST_FILE_LEN / 1024);
	runtime1 = bsp_GetRunTime();	/* 读取系统运行时间 */
	for (i = 0; i < TEST_FILE_LEN / BUF_SIZE; i++)
	{
		result = f_write(&file, g_TestBuf, sizeof(g_TestBuf), &bw);
		if (result == FR_OK)
		{
			if (((i + 1) % 8) == 0)
			{
				printf(".");
			}
		}
		else
		{
			err = 1;
			printf("%s文件写失败\r\n", TestFileName);
			break;
		}
	}
	runtime2 = bsp_GetRunTime();	/* 读取系统运行时间 */

	if (err == 0)
	{
		timelen = (runtime2 - runtime1);
		printf("\r\n  写耗时 : %dms   平均写速度 : %dB/S (%dKB/S)\r\n",
			timelen,
			(TEST_FILE_LEN * 1000) / timelen,
			((TEST_FILE_LEN / 1024) * 1000) / timelen);
	}

	f_close(&file);		/* 关闭文件*/


	/* 开始读文件测试 */
	result = f_open(&file, TestFileName, FA_OPEN_EXISTING | FA_READ);
	if (result !=  FR_OK)
	{
		printf("没有找到文件: %s\r\n", TestFileName);
		return;
	}

	printf("开始读文件 %dKB ...\r\n", TEST_FILE_LEN / 1024);
	runtime1 = bsp_GetRunTime();	/* 读取系统运行时间 */
	for (i = 0; i < TEST_FILE_LEN / BUF_SIZE; i++)
	{
		result = f_read(&file, g_TestBuf, sizeof(g_TestBuf), &bw);
		if (result == FR_OK)
		{
			if (((i + 1) % 8) == 0)
			{
				printf(".");
			}

			/* 比较写入的数据是否正确，此语句会导致读卡速度结果降低到 3.5MBytes/S */
			for (k = 0; k < sizeof(g_TestBuf); k++)
			{
				if (g_TestBuf[k] != (k / 512) + '0')
				{
				  	err = 1;
					printf("Speed1.txt 文件读成功，但是数据出错\r\n");
					break;
				}
			}
			if (err == 1)
			{
				break;
			}
		}
		else
		{
			err = 1;
			printf("Speed1.txt 文件读失败\r\n");
			break;
		}
	}
	runtime2 = bsp_GetRunTime();	/* 读取系统运行时间 */

	if (err == 0)
	{
		timelen = (runtime2 - runtime1);
		printf("\r\n  读耗时 : %dms   平均读速度 : %dB/S (%dKB/S)\r\n", timelen,
			(TEST_FILE_LEN * 1000) / timelen, ((TEST_FILE_LEN / 1024) * 1000) / timelen);
	}

	/* 关闭文件*/
	f_close(&file);

	/* 卸载文件系统 */
	f_mount(FS_USB, NULL);
}
