# STM32 HTTP客户端使用说明

## 项目概述

本项目已将STM32F407从HTTP服务器改造为HTTP客户端，实现单片机主动连接云端HTTP服务器进行通讯。

## 修改内容

### 1. 新增文件
- **USER/App/http_client.h** - HTTP客户端头文件
- **USER/App/http_client.c** - HTTP客户端实现文件

### 2. 修改文件
- **USER/main.c** - 主程序，使用HTTP客户端替代HTTP服务器

## 使用步骤

### 步骤1: 配置服务器地址

打开 `USER/App/http_client.h` 文件，修改以下配置：

```c
#define HTTP_SERVER_IP      "192.168.1.100"  // 修改为你的云端服务器IP地址
#define HTTP_SERVER_PORT    80                // 服务器端口
#define HTTP_REQUEST_PATH   "/api/data"       // 请求路径
```

**注意：** 如果要使用域名（如 www.example.com），需要：
1. 在 lwipopts.h 中启用 DNS: `#define LWIP_DNS 1`
2. 修改 http_client.c 中的连接函数使用 DNS 解析

### 步骤2: 修改本地IP地址（可选）

如果需要修改单片机的IP地址，打开 `USER/App/netconf.h`，查找IP地址定义并修改：

```c
#define IP_ADDR0   192
#define IP_ADDR1   168
#define IP_ADDR2   1
#define IP_ADDR3   10
```

### 步骤3: 编译和烧录

1. 在Keil工程中添加新文件：
   - 将 `http_client.c` 添加到工程中
   
2. 编译工程（无错误即可）

3. 烧录到STM32F407开发板

### 步骤4: 测试

1. **搭建测试服务器**（可使用以下任一方式）：

   **方式A - 使用Node.js快速搭建测试服务器：**
   ```javascript
   // test_server.js
   const http = require('http');
   
   const server = http.createServer((req, res) => {
       console.log(`收到请求: ${req.method} ${req.url}`);
       
       // 打印请求体
       let body = '';
       req.on('data', chunk => {
           body += chunk.toString();
       });
       
       req.on('end', () => {
           if (body) {
               console.log('请求数据:', body);
           }
           
           // 返回JSON响应
           res.writeHead(200, {'Content-Type': 'application/json'});
           res.end(JSON.stringify({
               status: 'success',
               message: '数据已接收',
               timestamp: new Date().toISOString()
           }));
       });
   });
   
   server.listen(80, '0.0.0.0', () => {
       console.log('测试服务器运行在 http://0.0.0.0:80');
   });
   ```
   
   运行: `node test_server.js`

   **方式B - 使用Python快速搭建：**
   ```python
   from http.server import HTTPServer, BaseHTTPRequestHandler
   import json
   
   class TestHandler(BaseHTTPRequestHandler):
       def do_GET(self):
           print(f"收到GET请求: {self.path}")
           self.send_response(200)
           self.send_header('Content-type', 'application/json')
           self.end_headers()
           response = {"status": "success", "method": "GET"}
           self.wfile.write(json.dumps(response).encode())
       
       def do_POST(self):
           content_length = int(self.headers['Content-Length'])
           post_data = self.rfile.read(content_length)
           print(f"收到POST请求: {self.path}")
           print(f"数据: {post_data.decode()}")
           
           self.send_response(200)
           self.send_header('Content-type', 'application/json')
           self.end_headers()
           response = {"status": "success", "method": "POST"}
           self.wfile.write(json.dumps(response).encode())
   
   server = HTTPServer(('0.0.0.0', 80), TestHandler)
   print('测试服务器运行在 http://0.0.0.0:80')
   server.serve_forever()
   ```
   
   运行: `python test_server.py`

2. **查看串口输出**：
   - 单片机会每5秒发送一次HTTP请求
   - 串口会打印连接状态、发送的请求、接收的响应

## 工作流程

```
1. 单片机启动
   ↓
2. 初始化网络 (LwIP)
   ↓
3. 初始化HTTP客户端
   ↓
4. 连接到云端服务器
   ↓
5. 发送HTTP GET/POST请求
   ↓
6. 接收服务器响应
   ↓
7. 每隔5秒重复步骤5-6
```

## API说明

### 主要函数

#### 1. `http_client_init()`
初始化HTTP客户端

#### 2. `http_client_connect()`
连接到HTTP服务器（使用http_client.h中配置的IP和端口）

#### 3. `http_client_send_get_request(path)`
发送HTTP GET请求
- **参数**: path - 请求路径，如 "/api/data"

#### 4. `http_client_send_post_request(path, data)`
发送HTTP POST请求
- **参数**: 
  - path - 请求路径
  - data - POST数据（通常为JSON格式字符串）

示例：
```c
const char *json_data = "{\"sensor\":\"temperature\",\"value\":25.5}";
http_client_send_post_request("/api/sensor", json_data);
```

#### 5. `http_client_get_state()`
获取客户端当前状态
- 返回值：
  - `HTTP_CLIENT_IDLE` - 空闲
  - `HTTP_CLIENT_CONNECTING` - 正在连接
  - `HTTP_CLIENT_CONNECTED` - 已连接
  - `HTTP_CLIENT_SENDING` - 正在发送
  - `HTTP_CLIENT_RECEIVING` - 正在接收
  - `HTTP_CLIENT_CLOSED` - 已关闭
  - `HTTP_CLIENT_ERROR` - 错误

#### 6. `http_client_get_response()`
获取服务器响应数据
- 返回值：响应数据字符串指针

## 自定义修改

### 修改请求间隔时间

在 `main.c` 中修改：
```c
#define HTTP_REQUEST_INTERVAL  5000  // 单位：毫秒
```

### 修改发送的数据内容

在 `main.c` 的主循环中修改：
```c
// 修改JSON数据内容
const char *json_data = "{\"sensor\":\"temperature\",\"value\":25.5}";
http_client_send_post_request("/api/sensor", json_data);
```

### 添加更多请求路径

可以在不同时间发送到不同路径：
```c
if (condition1) {
    http_client_send_get_request("/api/status");
} else {
    http_client_send_post_request("/api/data", json_data);
}
```

## 与云端服务器通讯示例

### 发送传感器数据到云端

```c
// 准备JSON数据
char json_buf[256];
float temperature = 25.5;
float humidity = 60.2;

snprintf(json_buf, sizeof(json_buf),
         "{\"device_id\":\"STM32_001\","
         "\"temperature\":%.1f,"
         "\"humidity\":%.1f,"
         "\"timestamp\":%lu}",
         temperature, humidity, LocalTime);

// 发送到云端
http_client_send_post_request("/api/sensor/upload", json_buf);
```

### 接收云端控制命令

在 `http_client.c` 的 `http_client_recv_cb()` 函数中，可以解析服务器返回的JSON数据来实现控制：

```c
// 示例：解析JSON响应控制LED
if (strstr(client->response_buf, "\"led\":\"on\"")) {
    LED1_ON;
} else if (strstr(client->response_buf, "\"led\":\"off\"")) {
    LED1_OFF;
}
```

## 常见问题

### Q1: 无法连接到服务器？
**解决方案：**
1. 检查服务器IP地址是否正确
2. 确认单片机和服务器在同一网络，或服务器端口已开放
3. 使用ping命令测试网络连通性
4. 检查防火墙设置

### Q2: 连接成功但无响应？
**解决方案：**
1. 检查服务器是否正常运行
2. 查看服务器日志是否收到请求
3. 确认请求路径是否正确

### Q3: 如何使用HTTPS？
**解决方案：**
HTTPS需要TLS/SSL支持，lwIP默认不包含。需要：
1. 集成mbedTLS库
2. 修改http_client使用TLS连接
3. 建议初学者先使用HTTP测试

### Q4: 如何使用域名而非IP？
**解决方案：**
1. 在 `lwipopts.h` 中添加：
   ```c
   #define LWIP_DNS 1
   ```
2. 修改 `http_client.c` 中的连接函数使用 `dns_gethostbyname()`

## 下一步扩展

1. **添加MQTT支持** - 用于物联网通讯
2. **添加JSON解析库** - 如cJSON，方便处理JSON数据
3. **添加WebSocket支持** - 实现双向实时通讯
4. **集成串口屏** - 按图片架构添加串口屏显示

## 技术支持

如有问题，请检查：
1. 串口输出的调试信息
2. 网络连接状态（LED指示）
3. 服务器端日志

---
**更新日期**: 2025-12-25
**适用平台**: STM32F407 + LAN8742A
**lwIP版本**: 1.4.1
