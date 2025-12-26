/**
  ******************************************************************************
  * @file    ota_update.h
  * @brief   OTA固件在线更新模块 - 从服务器下载并更新固件
  ******************************************************************************
  */

#ifndef __OTA_UPDATE_H
#define __OTA_UPDATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"
#include "lwip/tcp.h"

/* OTA配置 */
#define OTA_SERVER_IP           "192.168.1.100"  /* OTA服务器IP */
#define OTA_SERVER_PORT         80               /* OTA服务器端口 */
#define OTA_CHECK_URL           "/api/ota/check" /* 检查更新接口 */
#define OTA_DOWNLOAD_URL        "/api/ota/download" /* 下载固件接口 */

#define OTA_CHECK_INTERVAL      10000  /* 检查更新间隔(ms) */
#define OTA_BUFFER_SIZE         1024   /* 下载缓冲区大小 */
#define OTA_APP_START_ADDR      0x08010000  /* 应用程序起始地址 */
#define OTA_APP_MAX_SIZE        (384*1024)  /* 应用程序最大尺寸384KB */

#define CURRENT_VERSION         "1.0.0"  /* 当前固件版本 */

/* OTA状态 */
typedef enum {
    OTA_IDLE = 0,           /* 空闲 */
    OTA_CHECKING,           /* 检查更新中 */
    OTA_UPDATE_AVAILABLE,   /* 发现新版本 */
    OTA_DOWNLOADING,        /* 下载中 */
    OTA_DOWNLOAD_COMPLETE,  /* 下载完成 */
    OTA_FLASHING,           /* 烧写中 */
    OTA_SUCCESS,            /* 更新成功 */
    OTA_ERROR               /* 错误 */
} ota_state_t;

/* OTA信息结构体 */
typedef struct {
    ota_state_t state;
    char new_version[32];       /* 新版本号 */
    uint32_t firmware_size;     /* 固件大小 */
    uint32_t downloaded_size;   /* 已下载大小 */
    uint32_t flash_address;     /* 当前写入地址 */
    uint8_t progress;           /* 下载进度(0-100) */
    char error_msg[128];        /* 错误信息 */
} ota_info_t;

/* 函数声明 */
void ota_init(void);
void ota_task(void);
ota_state_t ota_get_state(void);
uint8_t ota_get_progress(void);
const char* ota_get_version(void);
const ota_info_t* ota_get_info(void);
void ota_start_update(void);
err_t ota_check_for_update(void);
err_t ota_download_firmware(void);

#ifdef __cplusplus
}
#endif

#endif /* __OTA_UPDATE_H */
