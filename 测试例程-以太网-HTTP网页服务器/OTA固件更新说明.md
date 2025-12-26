# STM32 OTA固件在线更新完整方案

## ? 功能概述

实现了完整的STM32固件OTA（Over-The-Air）更新方案：
- ? **网页管理界面** - 上传固件文件并发布更新
- ? **单片机自动检测** - 定期轮询服务器检查更新
- ? **分块下载** - 支持大文件分块传输
- ? **Flash写入** - 自动擦除并写入新固件
- ? **自动重启** - 下载完成后跳转到新程序

## ? 文件结构

```
项目目录/
├── USER/
│   ├── App/
│   │   ├── http_client.h/c      # HTTP客户端（已修改支持OTA）
│   │   ├── ota_update.h/c       # OTA更新模块（新增）
│   └── main.c                   # 主程序（已集成OTA）
├── ota_server.js                # Node.js OTA服务器
├── public/
│   └── index.html               # 网页管理界面
├── package.json                 # Node.js依赖配置
└── firmware/                    # 固件存放目录（自动创建）
```

## ? 快速开始

### 步骤1: 搭建OTA服务器

#### 方法A - 使用Node.js（推荐）

1. **安装Node.js**
   - 下载地址: https://nodejs.org/
   - 安装LTS版本

2. **安装依赖**
   ```bash
   cd 项目目录
   npm install
   ```

3. **启动服务器**
   ```bash
   npm start
   ```
   或
   ```bash
   node ota_server.js
   ```

4. **访问管理页面**
   - 打开浏览器访问: `http://localhost`

### 步骤2: 配置单片机

1. **修改服务器地址**
   
   打开 `USER/App/ota_update.h`，修改：
   ```c
   #define OTA_SERVER_IP       "192.168.1.100"  // 改为服务器IP
   #define OTA_CHECK_INTERVAL  10000            // 检查间隔(ms)
   ```

2. **添加文件到Keil工程**
   - 将 `ota_update.c` 添加到工程
   - 编译确认无错误

3. **烧录到单片机**

### 步骤3: 上传固件更新

1. **编译新版本固件**
   - 修改代码（添加新功能）
   - 在Keil中编译生成 `.bin` 文件
   - 路径通常在: `Output/项目名.bin`

2. **在网页上传固件**
   - 打开网页管理界面
   - 输入新版本号（如 `1.1.0`）
   - 选择编译好的 `.bin` 文件
   - 点击"上传并发布更新"

3. **等待单片机更新**
   - 单片机每10秒检查一次更新
   - 发现新版本后自动下载
   - 下载完成后自动重启

## ? 工作流程

```
┌─────────────┐         ┌─────────────┐         ┌─────────────┐
│   网页界面   │         │  OTA服务器  │         │   STM32单片机 │
└──────┬──────┘         └──────┬──────┘         └──────┬──────┘
       │                       │                       │
       │ 1. 上传.bin文件        │                       │
       ├──────────────────────>│                       │
       │                       │                       │
       │                       │  2. 定期检查更新      │
       │                       │<──────────────────────┤
       │                       │                       │
       │                       │  3. 返回版本信息      │
       │                       ├──────────────────────>│
       │                       │                       │
       │                       │  4. 请求下载固件      │
       │                       │<──────────────────────┤
       │                       │                       │
       │                       │  5. 发送固件数据      │
       │                       ├──────────────────────>│
       │                       │                       │
       │                       │  6. 继续下载下一块    │
       │                       │<──────────────────────┤
       │                       │     (循环直到完成)     │
       │                       │                       │
       │                       │                       ├─> 7. 写入Flash
       │                       │                       │
       │                       │                       ├─> 8. 重启运行新固件
```

## ? API接口说明

### 1. 检查更新
```
GET /api/ota/check?device_id=STM32_001&current_version=1.0.0

响应:
{
    "update_available": true,
    "version": "1.1.0",
    "size": 102400,
    "description": "新固件已上传"
}
```

### 2. 下载固件
```
GET /api/ota/download?device_id=STM32_001&offset=0

响应: 二进制固件数据（每次最多1KB）
```

### 3. 上传固件（网页调用）
```
POST /api/ota/upload
Content-Type: multipart/form-data

参数:
- version: 版本号
- firmware: .bin文件

响应:
{
    "success": true,
    "message": "固件上传成功",
    "version": "1.1.0",
    "size": 102400
}
```

## ? STM32端函数说明

### 主要函数

#### `ota_init()`
初始化OTA模块

#### `ota_task()`
OTA任务，需在主循环中调用，负责定期检查更新

#### `ota_check_for_update()`
手动检查是否有固件更新

#### `ota_start_update()`
开始下载并更新固件

#### `ota_get_state()`
获取当前OTA状态

#### `ota_get_progress()`
获取下载进度（0-100）

### 使用示例

```c
int main(void)
{
    // ... 初始化代码 ...
    
    ota_init();
    
    while(1)
    {
        // ... 网络处理 ...
        
        ota_task();  // 定期调用
        
        // 获取OTA状态
        if (ota_get_state() == OTA_DOWNLOADING) {
            printf("下载进度: %d%%\n", ota_get_progress());
        }
    }
}
```

## ?? 配置说明

### Flash分区

STM32F407默认配置：
```
0x08000000 - 0x0800FFFF (64KB)   : Bootloader区域
0x08010000 - 0x0806FFFF (384KB)  : 应用程序区域（OTA_APP_START_ADDR）
0x08070000 - 0x080FFFFF (576KB)  : 备用/数据区域
```

**重要：** 修改 `ota_update.h` 中的地址配置：
```c
#define OTA_APP_START_ADDR    0x08010000  // 应用起始地址
#define OTA_APP_MAX_SIZE      (384*1024)  // 最大尺寸
```

### 调整检查间隔

修改 `ota_update.h`：
```c
#define OTA_CHECK_INTERVAL  10000  // 单位：毫秒
```

### 修改下载块大小

修改 `ota_update.h`：
```c
#define OTA_BUFFER_SIZE  1024  // 单位：字节
```

## ? 调试与监控

### 串口输出信息

单片机会输出详细的OTA过程：
```
========================================
OTA更新模块初始化
当前版本: 1.0.0
应用起始地址: 0x08010000
========================================

[OTA] 检查固件更新...
[OTA] 发现新版本: 1.1.0
[OTA] ========== 开始固件更新 ==========
[OTA] 新版本: 1.1.0
[OTA] 固件大小: 102400 bytes
[OTA] 擦除Flash...
[OTA] 下载固件... (0/102400 bytes)
[OTA] 写入进度: 1% (1024/102400 bytes)
[OTA] 写入进度: 2% (2048/102400 bytes)
...
[OTA] ========== 下载完成 ==========
[OTA] 准备重启并更新固件...
```

### 服务器日志

服务器会显示所有请求：
```
[OTA检查] 设备: STM32_001, 当前版本: 1.0.0
[OTA检查] 发现新版本: 1.1.0
[OTA下载] 设备: STM32_001, 偏移: 0
[OTA下载] 发送数据: 0-1024/102400 bytes
[OTA下载] 发送数据: 1024-2048/102400 bytes
```

## ?? 安全建议

### 1. 固件验证（推荐添加）

在 `ota_update.c` 中添加CRC校验：
```c
// 服务器发送固件时包含CRC
// 单片机下载完成后验证CRC
uint32_t calculated_crc = calculate_crc32(firmware_data, size);
if (calculated_crc != received_crc) {
    printf("[OTA] CRC校验失败!\n");
    ota_info.state = OTA_ERROR;
}
```

### 2. 版本回退

保留旧固件在备份区，更新失败时可恢复

### 3. 安全连接

生产环境建议使用HTTPS（需要mbedTLS）

## ? 常见问题

### Q1: 如何生成.bin文件？

**Keil MDK:**
1. 选择 Options for Target
2. Output 选项卡
3. 勾选 "Create HEX File"
4. 使用 `arm-none-eabi-objcopy` 转换：
   ```bash
   arm-none-eabi-objcopy -O binary project.axf project.bin
   ```

或在Keil的 `Options -> User` 中添加：
```
After Build: fromelf --bin --output=.\Output\@L.bin .\Output\@L.axf
```

### Q2: Flash写入失败？

**检查：**
1. Flash地址是否正确
2. 是否成功解锁Flash
3. 是否擦除了目标扇区
4. 电压范围设置是否正确

### Q3: 下载很慢？

**优化方案：**
1. 增加下载块大小（`OTA_BUFFER_SIZE`）
2. 使用更快的网络连接
3. 服务器端启用gzip压缩

### Q4: 更新后程序不运行？

**检查：**
1. 链接器脚本中ROM起始地址是否为 `0x08010000`
2. 固件大小是否超出分配空间
3. 使用串口查看是否跳转成功

修改链接器脚本（.ld文件）：
```
MEMORY
{
    FLASH (rx)      : ORIGIN = 0x08010000, LENGTH = 384K
    RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 128K
}
```

### Q5: 如何测试OTA功能？

**测试步骤：**
1. 在代码中添加版本打印
2. 编译当前版本（v1.0.0）并烧录
3. 修改代码添加新功能
4. 编译新版本（v1.1.0）
5. 在网页上传新版本固件
6. 观察串口输出，确认下载和更新过程

## ? 未来扩展

### 1. 差分更新
只下载变化的部分，减少流量和时间

### 2. 断点续传
网络中断后可以从断点继续下载

### 3. 多设备管理
服务器支持管理多个设备的固件版本

### 4. 定时推送
服务器在指定时间主动推送更新

### 5. A/B分区
实现双分区，更新失败自动回滚

## ? 学习资源

- [lwIP官方文档](https://www.nongnu.org/lwip/)
- [STM32 Flash编程手册](https://www.st.com/resource/en/programming_manual/pm0081-stm32f40xxx-and-stm32f41xxx-flash-programming-manual-stmicroelectronics.pdf)
- [HTTP协议详解](https://developer.mozilla.org/zh-CN/docs/Web/HTTP)

---

**更新日期**: 2025-12-26  
**适用平台**: STM32F407 + LAN8742A  
**lwIP版本**: 1.4.1
