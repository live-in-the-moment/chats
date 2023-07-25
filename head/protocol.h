#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sqlite3.h>


// 功能指令
enum
{
    REG,              // 注册
    LOGIN,            // 登录
    FORGET,           // 忘记密码
    LOOKUSERS,        // 查看在线用户
    PRIVATE,          // 私聊
    GROUP,            // 群聊
    LOOKCHATRECORD,   // 查看聊天记录
    LOOKPMCHATRECORD, // 查询私聊记录
    QUIT,             // 退出处理
    // FILE,   // 传输文件
};

// 在线用户链表
typedef struct OnlineLinkList
{
    char id[128];
    char name[32];
    int cfd;
    int forbid_flag;
    char chat_status[16];        // 用户聊天状态
    struct OnlineLinkList *next; // 指针域，为了能够操作后面结点
                                 // 所以指针的类型为当前结构体的类型
} OnlineLinkList;


typedef struct thread_node
{
    int cfd;
    OnlineLinkList *head;
    sqlite3 *ppdb;
} thread_node;

// 保存信息的结构体
typedef struct Message
{
    struct
    {
        uint32_t crc32;  // CRC校验
        char sid[8];    // 账号
        char rid[8];
        int cfd;              // 聊天对象
        char msg_type[16];    // 消息类型
        char msg_time[32];    // 时间戳格式为YYYY-MM-DD HH:MM:SS
        char chat_status[16]; // 聊天状态
    } header;

    union
    {
        struct
        { // 登录请求消息
            // char username[32];  // 客户端用户名
            char password[32]; // 客户端密码
        } login_request;

        struct
        {                   // 响应消息
            int res_type;   // 响应类型
            char logs[512]; // 日志
        } response;

        struct
        {                         // 文件传输消息
            char file_path[1024]; // 文件路径
        } file_transfer;

        struct
        {                             // 在线列表消息
            int online_count;         // 当前在线客户端数量
            char online_clients[256]; // 各个客户端的用户名
        } online_list;

        struct
        {                 // 私聊响应消息
            int accepted; // 是否同意私聊的状态
        } private_chat_response;

        struct
        {                      // 聊天消息
            char content[512]; // 聊天内容
        } chat_message;

    } body;
} Message;




