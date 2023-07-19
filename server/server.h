#ifndef _SERVER_H_
#define _SERVER_H_
#include<sqlite3.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<errno.h>
#include<time.h>
#include<signal.h>
#include<arpa/inet.h>
#include <stdbool.h>

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
    // FILE,   // 传输文件
};

//在线用户链表
typedef struct OnlineLinkList
{
    char id[128];
    char name[32];
    int cfd;    
    int forbid_flag;  
    char chat_status[16]; //用户聊天状态
    struct OnlineLinkList *next; //指针域，为了能够操作后面结点
                                 //所以指针的类型为当前结构体的类型
}OnlineLinkList;

typedef struct thread_node
{
    int cfd;
    OnlineLinkList *head;
    sqlite3 *ppdb;
}thread_node;


// 保存信息的结构体
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
            char logs[512]; //日志
        } response;
        
        struct {  // 文件传输消息
            char file_path[1024];  // 文件路径
        } file_transfer;
        
        struct {  // 在线列表消息
            int online_count;  // 当前在线客户端数量
            char online_clients[256];  // 各个客户端的用户名
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


//创建一个空链表
void CreateLink(OnlineLinkList **head);
//创建新的结点
void CreateNode(OnlineLinkList ** new_node);
//头插法插入数据
void InsertNodeHead(OnlineLinkList *head, OnlineLinkList * new_node);
//遍历用户注册信息
void Id(sqlite3 * ppdb ) ;
//线程函数
void *MyFun(void *arg);
//查找id是否存在
int FindId(sqlite3 * ppdb , Message *data);
//注册
void Register(thread_node * node , Message *data); 
//向数据库中插入数据
void InsertData(sqlite3 *ppdb , Message *data);
//创建表
void CreatTable(sqlite3 *ppdb);
//收发消息
void MsgSendRecv(thread_node * cfd_node);
//查找账号和密保是否存在
int FindSecret(sqlite3 * ppdb , Message *data);
//更新数据库(更改密码)
void UpdateData(sqlite3 *ppdb , Message *data);
//登录
void Login(thread_node *cfd_node , Message *data);
//登录验证
int VerifyIdPassword(sqlite3 *ppdb , Message *data);
//群聊
void GroupChat(sqlite3 *ppdb , OnlineLinkList *head , Message *data);
//私聊
void PrivateChat(thread_node *node , Message *data);

//查看在线用户
int LookOnlineUsers(thread_node * node);
//检查自己是否在线
int InspectOwnOnline(thread_node *node);
//检查账号是否重复登录
int RepeatLogin(thread_node *node , Message *data);


//创建第二张表用于保存聊天记录
void CreatTable2(sqlite3 *ppdb);
// 创建第三张表用于保存私聊的记录
void CreatTable3(sqlite3 *ppdb);
//向第二张表中插入聊天记录
void InsertChatData(sqlite3 *ppdb , char *chat);
//向第三张表中插入聊天记录
void InsertPmChatData(sqlite3 *ppdb , char *chat, char *sid);
//遍历聊天记录
void  PrintChatRecord(sqlite3 *ppdb , thread_node *node);
// 遍历私聊天记录
void PrintPmChatRecord(sqlite3 *ppdb, thread_node *node, char *sid);
// 文件处理
void FileRecv(thread_node *node,Message *data);

//*********************************************
// 定长线程池实现
struct job                //存放线程函数，和传参
{
    void *(*func)(void *arg); //函数指针
    void *arg;
    struct job *next;
};

struct threadpool
{
    int thread_num;  //已开启线程池已工作线程
    pthread_t *pthread_ids;  // 薄脆线程池中线程id


    struct job *head;
    struct job *tail;  // 任务队列的尾
    int queue_max_num;  //任务队列的最多放多少个
    int queue_cur_num;  //任务队列已有多少个任务

    pthread_mutex_t mutex;
    pthread_cond_t queue_empty;    //任务队列为空
    pthread_cond_t queue_not_emtpy;  //任务队列不为空
    pthread_cond_t queue_not_full;  //任务队列不为满

    int pool_close;   //线程退出标志
};

void * threadpool_function(void *arg);//任务队列取数据 执行线程函数

struct threadpool * threadpool_init(int thread_num, int queue_max_num);

void threadpool_add_job(struct threadpool *pool, void *(*func)(void *), void *arg);//增加任务

void thread_destroy(struct threadpool *pool);

#endif