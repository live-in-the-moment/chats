#ifndef _CLIENT_H_
#define _CLIENT_H_
#include <sqlite3.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>
#include <signal.h>
#include <stdbool.h>

// 客户端套接字 socket
// int sockfd;

// 功能指令
enum {
    REG,    //注册
    LOGIN,  //登录
    FORGET, //忘记密码
    LOOKUSERS,  //查看在线用户
    PRIVATE,    //私聊
    GROUP,  //群聊
    LOOKCHATRECORD, //查看聊天记录
    LOOKPMCHATRECORD, //查询私聊记录
    QUIT, // 退出处理
    // FILE,   // 传输文件
};

typedef struct Message{
    struct {
		uint8_t T; // 
		uint32_t length; //数据长度
		uint32_t crc32; //CRC校验       
        char sid[32];  // 账号
        char rid[32]; 
        // char sender[32];  // 发送者用户名  
        // char receiver[32];  // 接收者用户名
        int cfd;    // 聊天对象
        char msg_type[16];  // 消息类型
        char msg_time[32];  // 时间戳格式为YYYY-MM-DD HH:MM:SS
        char chat_status[16]; //聊天状态
    } header;
    
    union {
        struct {  // 登录请求消息
            // char username[32];  // 客户端用户名
            char password[32];  // 客户端密码
        } login_request;
        
        struct {  // 响应消息
            int res_type;  // 响应类型
            char logs[64]; //日志
        } response;
        
        struct {  // 文件传输消息
            char file_path[1024];  // 文件路径
        } file_transfer;
        
        struct {  // 在线列表消息
            int online_count;  // 当前在线客户端数量
            char online_clients[128];  // 各个客户端的用户名
        } online_list;
        
        struct {  // 私聊响应消息
            int accepted;  // 是否同意私聊的状态
        } private_chat_response;
        
        struct {  // 聊天消息
            char content[512];  // 聊天内容
        } chat_message;
        
        struct {  // 心跳消息
            bool online_status;  // 客户端在线状态
        } heartbeat;
    } body;
} Message;





//聊天室功能能选择界面
void menu();
//写线程 
void *write_thread(void * arg);
//读线程 
void *read_thread(void * arg);
//修改退出聊天室的方式
void Close(int signum);

void file_from(int sockfd);

void file_recv(char buffer[]);


#endif