#ifndef _VERSION_H_
#define _VERSION_H_
#include "main.h"

/*=========================== 固件版本信息 ===========================*/
#define FIRMWARE_VERSION_MAJOR    1      /* 主版本号 */
#define FIRMWARE_VERSION_MINOR    0      /* 次版本号 */
#define FIRMWARE_VERSION_PATCH    1      /* 修订号 */
#define FIRMWARE_NAME             "更新数据写入SD卡"  /* 固件名称 */

/* 固件信息结构体 */
typedef struct {
    const char *name;           /* 固件名称 */
    uint8_t version_major;      /* 主版本号 */
    uint8_t version_minor;      /* 次版本号 */
    uint8_t version_patch;      /* 修订号 */
    const char *build_date;     /* 编译日期 */
    const char *build_time;     /* 编译时间 */
} FirmwareInfo_t;

/* 外部变量声明 */
extern const FirmwareInfo_t g_FirmwareInfo;

/* 函数声明 */
void PrintFirmwareInfo(void);

#endif
