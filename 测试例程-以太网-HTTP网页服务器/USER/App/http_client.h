/**
  ******************************************************************************
  * @file    http_client.h
  * @brief   HTTP客户端头文件 - 用于向云端服务器发送HTTP请求
  ******************************************************************************
  */

#ifndef __HTTP_CLIENT_H
#define __HTTP_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lwip/tcp.h"

/* HTTP客户端配置 */
#define HTTP_SERVER_IP      "192.168.1.100"  /* 云端服务器IP地址，需要修改为实际服务器地址 */
#define HTTP_SERVER_PORT    80                /* 服务器端口 */
#define HTTP_REQUEST_PATH   "/api/data"       /* 请求路径 */

/* HTTP客户端状态 */
typedef enum {
    HTTP_CLIENT_IDLE = 0, // 空闲
    HTTP_CLIENT_CONNECTING, // 正在连接
    HTTP_CLIENT_CONNECTED, // 已连接
    HTTP_CLIENT_SENDING, // 正在发送
    HTTP_CLIENT_RECEIVING, // 正在接收
    HTTP_CLIENT_CLOSED, // 已关闭
    HTTP_CLIENT_ERROR // 错误
} http_client_state_t;

/* HTTP客户端结构体 */
typedef struct {
    struct tcp_pcb *pcb;
    http_client_state_t state;
    char *request_data;      /* 要发送的数据 */
    uint16_t request_len;    /* 数据长度 */
    char response_buf[2048]; /* 接收缓冲区 */
    uint16_t response_len;   /* 已接收数据长度 */
} http_client_t;

/* 函数声明 */
void http_client_init(void);
err_t http_client_connect(void);
err_t http_client_send_get_request(const char *path);
err_t http_client_send_post_request(const char *path, const char *data);
void http_client_close(void);
http_client_state_t http_client_get_state(void);
const char* http_client_get_response(void);

#ifdef __cplusplus
}
#endif

#endif /* __HTTP_CLIENT_H */
