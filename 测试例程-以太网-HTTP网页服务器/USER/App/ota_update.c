/**
  ******************************************************************************
  * @file    ota_update.c
  * @brief   OTA固件在线更新实现
  ******************************************************************************
  */

#include "ota_update.h"
#include "http_client.h"
#include "stm32f4xx_flash.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* 全局OTA信息 */
static ota_info_t ota_info;
static uint32_t last_check_time = 0;
static uint8_t download_buffer[OTA_BUFFER_SIZE];

/* 内部函数声明 */
static void ota_erase_flash(void);
static uint8_t ota_write_flash(uint32_t addr, uint8_t *data, uint32_t len);
static void ota_jump_to_app(void);
static uint8_t ota_parse_check_response(const char *response);
static uint8_t ota_parse_firmware_data(const char *response, uint32_t *data_len);

/**
  * @brief  初始化OTA模块
  */
void ota_init(void)
{
    memset(&ota_info, 0, sizeof(ota_info_t));
    ota_info.state = OTA_IDLE;
    strncpy(ota_info.new_version, CURRENT_VERSION, sizeof(ota_info.new_version));
    
    printf("\n========================================\n");
    printf("OTA更新模块初始化\n");
    printf("当前版本: %s\n", CURRENT_VERSION);
    printf("应用起始地址: 0x%08X\n", OTA_APP_START_ADDR);
    printf("========================================\n");
}

/**
  * @brief  OTA任务，需要在主循环中周期调用
  */
void ota_task(void)
{
    extern __IO uint32_t LocalTime;
    
    /* 定期检查更新 */
    if (ota_info.state == OTA_IDLE) {
        if ((LocalTime - last_check_time) > OTA_CHECK_INTERVAL) {
            last_check_time = LocalTime;
            printf("\n[OTA] 检查固件更新...\n");
            ota_check_for_update();
        }
    }
}

/**
  * @brief  检查是否有固件更新
  * @retval err_t 错误代码
  */
err_t ota_check_for_update(void)
{
    char request[256];
    err_t err;
    
    ota_info.state = OTA_CHECKING;
    
    /* 确保HTTP客户端已连接 */
    if (http_client_get_state() != HTTP_CLIENT_CONNECTED) {
        printf("[OTA] 正在连接服务器...\n");
        err = http_client_connect();
        if (err != ERR_OK) {
            ota_info.state = OTA_IDLE;
            return err;
        }
        /* 等待连接建立 */
        return ERR_INPROGRESS;
    }
    
    /* 发送检查更新请求 */
    snprintf(request, sizeof(request), 
             "%s?device_id=STM32_001&current_version=%s", 
             OTA_CHECK_URL, CURRENT_VERSION);
    
    err = http_client_send_get_request(request);
    
    if (err == ERR_OK) {
        printf("[OTA] 检查请求已发送\n");
        /* 需要在HTTP响应回调中解析结果 */
    } else {
        printf("[OTA] 检查请求发送失败\n");
        ota_info.state = OTA_IDLE;
    }
    
    return err;
}

/**
  * @brief  开始下载并更新固件
  */
void ota_start_update(void)
{
    printf("\n[OTA] ========== 开始固件更新 ==========\n");
    printf("[OTA] 新版本: %s\n", ota_info.new_version);
    printf("[OTA] 固件大小: %lu bytes\n", ota_info.firmware_size);
    
    ota_info.state = OTA_DOWNLOADING;
    ota_info.downloaded_size = 0;
    ota_info.progress = 0;
    ota_info.flash_address = OTA_APP_START_ADDR;
    
    /* 擦除Flash */
    printf("[OTA] 擦除Flash...\n");
    ota_erase_flash();
    
    /* 开始下载固件 */
    ota_download_firmware();
}

/**
  * @brief  下载固件
  * @retval err_t 错误代码
  */
err_t ota_download_firmware(void)
{
    char request[256];
    err_t err;
    
    /* 构造下载请求，可以支持分块下载 */
    snprintf(request, sizeof(request),
             "%s?device_id=STM32_001&offset=%lu",
             OTA_DOWNLOAD_URL, ota_info.downloaded_size);
    
    printf("[OTA] 下载固件... (%lu/%lu bytes)\n",
           ota_info.downloaded_size, ota_info.firmware_size);
    
    err = http_client_send_get_request(request);
    
    return err;
}

/**
  * @brief  处理下载的固件数据（在HTTP接收回调中调用）
  * @param  data: 固件数据
  * @param  len: 数据长度
  * @retval 1=成功, 0=失败
  */
uint8_t ota_process_firmware_data(uint8_t *data, uint32_t len)
{
    if (ota_info.state != OTA_DOWNLOADING) {
        return 0;
    }
    
    /* 写入Flash */
    if (!ota_write_flash(ota_info.flash_address, data, len)) {
        printf("[OTA] Flash写入失败!\n");
        strcpy(ota_info.error_msg, "Flash写入失败");
        ota_info.state = OTA_ERROR;
        return 0;
    }
    
    /* 更新进度 */
    ota_info.downloaded_size += len;
    ota_info.flash_address += len;
    ota_info.progress = (ota_info.downloaded_size * 100) / ota_info.firmware_size;
    
    printf("[OTA] 写入进度: %d%% (%lu/%lu bytes)\n",
           ota_info.progress, ota_info.downloaded_size, ota_info.firmware_size);
    
    /* 检查是否下载完成 */
    if (ota_info.downloaded_size >= ota_info.firmware_size) {
        ota_info.state = OTA_DOWNLOAD_COMPLETE;
        printf("\n[OTA] ========== 下载完成 ==========\n");
        printf("[OTA] 准备重启并更新固件...\n");
        
        /* 延时后跳转到新程序 */
        // Delay_ms(2000);
        // ota_jump_to_app();
        
        return 1;
    }
    
    /* 继续下载下一块 */
    if (ota_info.downloaded_size < ota_info.firmware_size) {
        ota_download_firmware();
    }
    
    return 1;
}

/**
  * @brief  获取OTA状态
  */
ota_state_t ota_get_state(void)
{
    return ota_info.state;
}

/**
  * @brief  获取下载进度
  */
uint8_t ota_get_progress(void)
{
    return ota_info.progress;
}

/**
  * @brief  获取版本号
  */
const char* ota_get_version(void)
{
    return ota_info.new_version;
}

/**
  * @brief  获取OTA信息
  */
const ota_info_t* ota_get_info(void)
{
    return &ota_info;
}

/* ==================== Flash操作函数 ==================== */

/**
  * @brief  擦除应用程序Flash区域
  */
static void ota_erase_flash(void)
{
    uint32_t sector_error = 0;
    FLASH_Status status;
    
    /* 解锁Flash */
    FLASH_Unlock();
    
    /* 清除标志位 */
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                    FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
    
    /* 擦除扇区4-11 (0x08010000 - 0x080FFFFF, 共384KB) */
    /* STM32F407: Sector 4=64KB, 5-11=128KB each */
    printf("[OTA] 擦除Sector 4...\n");
    status = FLASH_EraseSector(FLASH_Sector_4, VoltageRange_3);
    if (status != FLASH_COMPLETE) {
        printf("[OTA] 擦除Sector 4 失败!\n");
    }
    
    printf("[OTA] 擦除Sector 5...\n");
    status = FLASH_EraseSector(FLASH_Sector_5, VoltageRange_3);
    if (status != FLASH_COMPLETE) {
        printf("[OTA] 擦除Sector 5 失败!\n");
    }
    
    /* 根据需要擦除更多扇区 */
    
    /* 锁定Flash */
    FLASH_Lock();
    
    printf("[OTA] Flash擦除完成\n");
}

/**
  * @brief  写入数据到Flash
  * @param  addr: 目标地址
  * @param  data: 数据指针
  * @param  len: 数据长度
  * @retval 1=成功, 0=失败
  */
static uint8_t ota_write_flash(uint32_t addr, uint8_t *data, uint32_t len)
{
    uint32_t i;
    FLASH_Status status;
    
    /* 解锁Flash */
    FLASH_Unlock();
    
    /* 按字节写入 */
    for (i = 0; i < len; i++) {
        status = FLASH_ProgramByte(addr + i, data[i]);
        if (status != FLASH_COMPLETE) {
            FLASH_Lock();
            return 0;
        }
    }
    
    /* 锁定Flash */
    FLASH_Lock();
    
    return 1;
}

/**
  * @brief  跳转到应用程序
  */
static void ota_jump_to_app(void)
{
    typedef void (*pFunction)(void);
    uint32_t jump_addr;
    pFunction jump_to_application;
    
    printf("[OTA] 跳转到新程序...\n");
    
    /* 检查栈顶地址是否合法 */
    if (((*(__IO uint32_t*)OTA_APP_START_ADDR) & 0x2FFE0000) == 0x20000000) {
        /* 跳转地址 = 复位向量地址 */
        jump_addr = *(__IO uint32_t*)(OTA_APP_START_ADDR + 4);
        jump_to_application = (pFunction)jump_addr;
        
        /* 初始化应用程序栈指针 */
        __set_MSP(*(__IO uint32_t*)OTA_APP_START_ADDR);
        
        /* 跳转 */
        jump_to_application();
    } else {
        printf("[OTA] 应用程序无效!\n");
        ota_info.state = OTA_ERROR;
    }
}

/**
  * @brief  解析检查更新响应（需在http_client.c的接收回调中调用）
  * @param  response: HTTP响应
  * @retval 1=有更新, 0=无更新
  */
uint8_t ota_parse_check_response(const char *response)
{
    /* 简单的JSON解析示例
     * 响应格式: {"update_available":true,"version":"1.1.0","size":102400}
     */
    
    if (strstr(response, "\"update_available\":true")) {
        char *version_ptr = strstr(response, "\"version\":\"");
        char *size_ptr = strstr(response, "\"size\":");
        
        if (version_ptr) {
            version_ptr += 11; /* 跳过 "version":" */
            sscanf(version_ptr, "%31[^\"]", ota_info.new_version);
            printf("[OTA] 发现新版本: %s\n", ota_info.new_version);
        }
        
        if (size_ptr) {
            size_ptr += 7; /* 跳过 "size": */
            ota_info.firmware_size = atoi(size_ptr);
            printf("[OTA] 固件大小: %lu bytes\n", ota_info.firmware_size);
        }
        
        ota_info.state = OTA_UPDATE_AVAILABLE;
        return 1;
    }
    
    printf("[OTA] 当前已是最新版本\n");
    ota_info.state = OTA_IDLE;
    return 0;
}
