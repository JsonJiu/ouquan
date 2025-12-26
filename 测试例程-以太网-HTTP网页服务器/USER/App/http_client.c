/**
  ******************************************************************************
  * @file    http_client.c
  * @brief   HTTP客户端实现 - 使用lwIP TCP连接云端服务器
  ******************************************************************************
  */

#include "http_client.h"
#include "lwip/tcp.h"
#include "lwip/dns.h"
#include "lwip/ip_addr.h"
#include <string.h>
#include <stdio.h>

/* 外部函数声明 - OTA相关 */
extern uint8_t ota_parse_check_response(const char *response);
extern uint8_t ota_process_firmware_data(uint8_t *data, uint32_t len);
extern void ota_start_update(void);

/* 全局HTTP客户端实例 */
static http_client_t http_client;

/* 内部函数声明 */
static err_t http_client_connected_cb(void *arg, struct tcp_pcb *tpcb, err_t err);
static err_t http_client_recv_cb(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t http_client_sent_cb(void *arg, struct tcp_pcb *tpcb, u16_t len);
static void http_client_error_cb(void *arg, err_t err);

/**
  * @brief  初始化HTTP客户端
  * @param  无
  * @retval 无
  */
void http_client_init(void)
{
    memset(&http_client, 0, sizeof(http_client_t));
    http_client.state = HTTP_CLIENT_IDLE;
    http_client.pcb = NULL;
    
    printf("HTTP客户端初始化完成\n");
}

/**
  * @brief  连接到HTTP服务器
  * @param  无
  * @retval err_t 错误代码
  */
err_t http_client_connect(void)
{
    ip_addr_t server_ip;
    err_t err;
    
    /* 如果已经连接，先关闭 */
    if (http_client.pcb != NULL) {
        http_client_close();
    }
    
    /* 创建新的TCP控制块 */
    http_client.pcb = tcp_new();
    if (http_client.pcb == NULL) {
        printf("创建TCP PCB失败\n");
        http_client.state = HTTP_CLIENT_ERROR;
        return ERR_MEM;
    }
    
    /* 解析服务器IP地址 */
    /* 注意：这里使用静态IP，如需使用域名请使用DNS解析 */
    IP4_ADDR(&server_ip, 192, 168, 1, 100); /* 修改为实际服务器IP */
    
    /* 绑定回调函数 */
    tcp_arg(http_client.pcb, &http_client);
    tcp_err(http_client.pcb, http_client_error_cb);
    
    /* 连接到服务器 */
    printf("正在连接HTTP服务器 %d.%d.%d.%d:%d...\n",
           ip4_addr1(&server_ip), ip4_addr2(&server_ip),
           ip4_addr3(&server_ip), ip4_addr4(&server_ip),
           HTTP_SERVER_PORT);
    
    http_client.state = HTTP_CLIENT_CONNECTING;
    err = tcp_connect(http_client.pcb, &server_ip, HTTP_SERVER_PORT, http_client_connected_cb);
    
    if (err != ERR_OK) {
        printf("连接失败: %d\n", err);
        tcp_abort(http_client.pcb);
        http_client.pcb = NULL;
        http_client.state = HTTP_CLIENT_ERROR;
        return err;
    }
    
    return ERR_OK;
}

/**
  * @brief  发送HTTP GET请求
  * @param  path: 请求路径 (例如: "/api/data")
  * @retval err_t 错误代码
  */
err_t http_client_send_get_request(const char *path)
{
    char request[512];
    int len;
    err_t err;
    
    if (http_client.state != HTTP_CLIENT_CONNECTED) {
        printf("未连接到服务器\n");
        return ERR_CONN;
    }
    
    /* 构造HTTP GET请求 */
    len = snprintf(request, sizeof(request),
                   "GET %s HTTP/1.1\r\n"
                   "Host: %s\r\n"
                   "Connection: keep-alive\r\n"
                   "User-Agent: STM32-HTTP-Client/1.0\r\n"
                   "\r\n",
                   path, HTTP_SERVER_IP);
    
    printf("发送HTTP GET请求:\n%s", request);
    
    /* 发送请求 */
    http_client.state = HTTP_CLIENT_SENDING;
    err = tcp_write(http_client.pcb, request, len, TCP_WRITE_FLAG_COPY);
    
    if (err == ERR_OK) {
        err = tcp_output(http_client.pcb);
        if (err == ERR_OK) {
            printf("请求发送成功\n");
        } else {
            printf("发送失败: %d\n", err);
            http_client.state = HTTP_CLIENT_ERROR;
        }
    } else {
        printf("写入失败: %d\n", err);
        http_client.state = HTTP_CLIENT_ERROR;
    }
    
    return err;
}

/**
  * @brief  发送HTTP POST请求
  * @param  path: 请求路径
  * @param  data: POST数据 (JSON格式等)
  * @retval err_t 错误代码
  */
err_t http_client_send_post_request(const char *path, const char *data)
{
    char request[1024];
    int len;
    int data_len = strlen(data);
    err_t err;
    
    if (http_client.state != HTTP_CLIENT_CONNECTED) {
        printf("未连接到服务器\n");
        return ERR_CONN;
    }
    
    /* 构造HTTP POST请求 */
    len = snprintf(request, sizeof(request),
                   "POST %s HTTP/1.1\r\n"
                   "Host: %s\r\n"
                   "Content-Type: application/json\r\n"
                   "Content-Length: %d\r\n"
                   "Connection: keep-alive\r\n"
                   "User-Agent: STM32-HTTP-Client/1.0\r\n"
                   "\r\n"
                   "%s",
                   path, HTTP_SERVER_IP, data_len, data);
    
    printf("发送HTTP POST请求:\n%s", request);
    
    /* 发送请求 */
    http_client.state = HTTP_CLIENT_SENDING;
    err = tcp_write(http_client.pcb, request, len, TCP_WRITE_FLAG_COPY);
    
    if (err == ERR_OK) {
        err = tcp_output(http_client.pcb);
        if (err == ERR_OK) {
            printf("请求发送成功\n");
        } else {
            printf("发送失败: %d\n", err);
            http_client.state = HTTP_CLIENT_ERROR;
        }
    } else {
        printf("写入失败: %d\n", err);
        http_client.state = HTTP_CLIENT_ERROR;
    }
    
    return err;
}

/**
  * @brief  关闭HTTP客户端连接
  * @param  无
  * @retval 无
  */
void http_client_close(void)
{
    if (http_client.pcb != NULL) {
        tcp_arg(http_client.pcb, NULL);
        tcp_sent(http_client.pcb, NULL);
        tcp_recv(http_client.pcb, NULL);
        tcp_err(http_client.pcb, NULL);
        
        tcp_close(http_client.pcb);
        http_client.pcb = NULL;
    }
    
    http_client.state = HTTP_CLIENT_CLOSED;
    printf("HTTP客户端连接已关闭\n");
}

/**
  * @brief  获取客户端状态
  * @param  无
  * @retval http_client_state_t 当前状态
  */
http_client_state_t http_client_get_state(void)
{
    return http_client.state;
}

/**
  * @brief  获取服务器响应数据
  * @param  无
  * @retval const char* 响应数据指针
  */
const char* http_client_get_response(void)
{
    return http_client.response_buf;
}

/* ==================== 内部回调函数 ==================== */

/**
  * @brief  TCP连接建立回调函数
  * @param  arg: 用户参数
  * @param  tpcb: TCP控制块
  * @param  err: 错误代码
  * @retval err_t 错误代码
  */
static err_t http_client_connected_cb(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    http_client_t *client = (http_client_t *)arg;
    
    if (err == ERR_OK) {
        printf("成功连接到HTTP服务器\n");
        client->state = HTTP_CLIENT_CONNECTED;
        
        /* 设置接收和发送回调 */
        tcp_recv(tpcb, http_client_recv_cb);
        tcp_sent(tpcb, http_client_sent_cb);
        
        /* 连接成功后，可以自动发送第一个请求 */
        /* http_client_send_get_request("/api/data"); */
    } else {
        printf("连接失败: %d\n", err);
        client->state = HTTP_CLIENT_ERROR;
        tcp_abort(tpcb);
        client->pcb = NULL;
    }
    
    return ERR_OK;
}

/**
  * @brief  TCP数据接收回调函数
  * @param  arg: 用户参数
  * @param  tpcb: TCP控制块
  * @param  p: 接收到的数据包
  * @param  err: 错误代码
  * @retval err_t 错误代码
  */
static err_t http_client_recv_cb(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    http_client_t *client = (http_client_t *)arg;
    struct pbuf *q;
    uint16_t copy_len;
    
    /* 连接被对方关闭 */
    if (p == NULL) {
        printf("服务器关闭连接\n");
        http_client_close();
        return ERR_OK;
    }
    
    if (err == ERR_OK) {
        client->state = HTTP_CLIENT_RECEIVING;
        
        /* 复制数据到响应缓冲区 */
        for (q = p; q != NULL; q = q->next) {
            copy_len = q->len;
            if (client->response_len + copy_len > sizeof(client->response_buf) - 1) {
                copy_len = sizeof(client->response_buf) - 1 - client->response_len;
            }
            
            if (copy_len > 0) {
                memcpy(client->response_buf + client->response_len, q->payload, copy_len);
                client->response_len += copy_len;
            }
        }
        
        client->response_buf[client->response_len] = '\0';
        
        printf("收到HTTP响应 (%d bytes):\n%s\n", client->response_len, client->response_buf);
        
        /* 检查是否是OTA相关响应 */
        if (strstr(client->response_buf, "/api/ota/check")) {
            /* OTA检查响应 */
            if (ota_parse_check_response(client->response_buf)) {
                printf("[HTTP] 发现固件更新，准备下载...\n");
                ota_start_update();
            }
        }
        else if (strstr(client->response_buf, "/api/ota/download")) {
            /* OTA固件数据响应 - 查找HTTP body */
            char *body = strstr(client->response_buf, "\r\n\r\n");
            if (body) {
                body += 4; /* 跳过\r\n\r\n */
                uint32_t body_len = client->response_len - (body - client->response_buf);
                printf("[HTTP] 收到固件数据: %lu bytes\n", body_len);
                ota_process_firmware_data((uint8_t*)body, body_len);
            }
        }
        
        /* 通知lwIP我们已经处理了数据 */
        tcp_recved(tpcb, p->tot_len);
        
        /* 释放pbuf */
        pbuf_free(p);
        
        client->state = HTTP_CLIENT_CONNECTED;
    }
    
    return ERR_OK;
}

/**
  * @brief  TCP数据发送完成回调函数
  * @param  arg: 用户参数
  * @param  tpcb: TCP控制块
  * @param  len: 已发送的字节数
  * @retval err_t 错误代码
  */
static err_t http_client_sent_cb(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    http_client_t *client = (http_client_t *)arg;
    
    printf("数据发送完成: %d bytes\n", len);
    
    if (client->state == HTTP_CLIENT_SENDING) {
        client->state = HTTP_CLIENT_CONNECTED;
    }
    
    return ERR_OK;
}

/**
  * @brief  TCP错误回调函数
  * @param  arg: 用户参数
  * @param  err: 错误代码
  * @retval 无
  */
static void http_client_error_cb(void *arg, err_t err)
{
    http_client_t *client = (http_client_t *)arg;
    
    printf("TCP错误: %d\n", err);
    
    /* 不要在这里调用tcp_abort或tcp_close，因为连接已经被关闭了 */
    client->pcb = NULL;
    client->state = HTTP_CLIENT_ERROR;
}
