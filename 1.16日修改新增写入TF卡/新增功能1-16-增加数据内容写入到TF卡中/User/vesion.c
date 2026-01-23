
#include "main.h"
#include "version.h"

/*=========================== 固件版本信息 ===========================*/
/* 固件信息定义（存储在Flash中） */
const FirmwareInfo_t g_FirmwareInfo __attribute__((used)) = {
    .name = FIRMWARE_NAME,
    .version_major = FIRMWARE_VERSION_MAJOR,
    .version_minor = FIRMWARE_VERSION_MINOR,
    .version_patch = FIRMWARE_VERSION_PATCH,
    .build_date = __DATE__,    /* 编译器自动填充编译日期，格式: "Jan 19 2026" */
    .build_time = __TIME__     /* 编译器自动填充编译时间，格式: "14:30:00" */
};

/*
*********************************************************************************************************
*	函 数 名: PrintFirmwareInfo
*	功能说明: 打印固件版本信息
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void PrintFirmwareInfo(void)
{
    printf("\r\n");
    printf("============================================\r\n");
    printf("       固件版本信息\r\n");
    printf("============================================\r\n");
    printf("固件名称: %s\r\n", g_FirmwareInfo.name);
    printf("版 本 号: V%d.%d.%d\r\n", 
           g_FirmwareInfo.version_major,
           g_FirmwareInfo.version_minor,
           g_FirmwareInfo.version_patch);
    printf("编译日期: %s\r\n", g_FirmwareInfo.build_date);
    printf("编译时间: %s\r\n", g_FirmwareInfo.build_time);
    printf("============================================\r\n");
    printf("\r\n");
}

