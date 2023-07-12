#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>

// #define PORT 8080
// #define SEM_SIZE 10

typedef struct Message{
    struct {
		uint8_t T; // 
		uint32_t length; //数据长度
		uint32_t crc32; //CRC校验       
        int sid[32];  
        char sender[32];  // 发送者用户名  
        int rid[32];
        char receiver[32];  // 接收者用户名
        int cfd[32];
        char message_type[32];  // 消息类型
        char msg_time[32];  // 时间戳格式为YYYY-MM-DD HH:MM:SS
    } header;
    
    union {
        struct {  // 登录请求消息
            // char username[32];  // 客户端用户名
            char password[32];  // 客户端密码
        } login_request;
        
        struct {  // 异常响应消息
            bool status;  // 是否成功的信息
        } login_response;
        
        struct {  // 文件传输消息
            char file_path[256];  // 文件路径
        } file_transfer;
        
        struct {  // 在线列表消息
            int online_count;  // 当前在线客户端数量
            char online_clients[256];  // 各个客户端的用户名
        } online_list;
        
        struct {  // 私聊响应消息
            bool accepted;  // 是否同意私聊的状态
        } private_chat_response;
        
        struct {  // 聊天消息
            char content[256];  // 聊天内容
        } chat_message;
        
        struct {  // 心跳消息
            bool online_status;  // 客户端在线状态
        } heartbeat;
    } body;
} Message;










// Packet packet;
// memset(&packet, 0, sizeof(packet));  // 初始化 packet 对象为全零
// strcpy(packet.header.sender, "Alice");
// strcpy(packet.header.receiver, "Bob");
// strcpy(packet.header.message_type, "chat");
// strcpy(packet.header.timestamp, "2023-07-11 10:23:45");

// packet.body.chat_message.sender = "Alice";
// strcpy(packet.body.chat_message.content, "Hello, how are you?");
// strcpy(packet.body.chat_message.timestamp, "2023-07-11 10:23:46");

// packet.length = sizeof(packet);