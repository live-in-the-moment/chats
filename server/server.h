#ifndef _SERVER_H_
#define _SERVER_H_
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
#include <time.h>
#include <signal.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "../head/protocol.h"

// 创建一个空链表
void CreateLink(OnlineLinkList **head);
// 创建新的结点
void CreateNode(OnlineLinkList **new_node);
// 头插法插入数据
void InsertNodeHead(OnlineLinkList *head, OnlineLinkList *new_node);
// 遍历用户注册信息
void Id(sqlite3 *ppdb);
// 线程函数
void *MyFun(void *arg);
// 查找id是否存在
int FindId(sqlite3 *ppdb, Message *data);
// 注册
void Register(thread_node *node, Message *data);
// 向数据库中插入数据
void InsertData(sqlite3 *ppdb, Message *data);
// 创建表
void CreatTable(sqlite3 *ppdb);
// 收发消息
void MsgSendRecv(thread_node *cfd_node);
// 查找账号和密保是否存在
int FindSecret(sqlite3 *ppdb, Message *data);
// 更新数据库(更改密码)
void UpdateData(sqlite3 *ppdb, Message *data);
// 登录
void Login(thread_node *cfd_node, Message *data);
// 登录验证
int VerifyIdPassword(sqlite3 *ppdb, Message *data);
// 群聊
void GroupChat(sqlite3 *ppdb, OnlineLinkList *head, Message *data);
// 私聊
void PrivateChat(thread_node *node, Message *data);
// 退出程序
void AllChat(thread_node *node, Message *data,char *msg);
// 心跳监听
void CreateHeartbeat(thread_node *node);
// 查看在线用户
int LookOnlineUsers(thread_node *node);
// 检查自己是否在线
int InspectOwnOnline(thread_node *node);
// 检查账号是否重复登录
int RepeatLogin(thread_node *node, Message *data);

// 创建第二张表用于保存聊天记录
void CreatTable2(sqlite3 *ppdb);
// 创建第三张表用于保存私聊的记录
void CreatTable3(sqlite3 *ppdb);
// 向第二张表中插入聊天记录
void InsertChatData(sqlite3 *ppdb, char *chat);
// 向第三张表中插入聊天记录
void InsertPmChatData(sqlite3 *ppdb, char *chat, char *sid);
// 遍历聊天记录
void PrintChatRecord(sqlite3 *ppdb, thread_node *node);
// 遍历私聊天记录
void PrintPmChatRecord(sqlite3 *ppdb, thread_node *node, char *sid);
// 文件处理
void FileRecv(thread_node *node, Message *data);

//*********************************************
// 定长线程池实现
struct job // 存放线程函数，和传参
{
    void *(*func)(void *arg); // 函数指针
    void *arg;
    struct job *next;
};

struct threadpool
{
    int thread_num;         // 已开启线程池已工作线程
    pthread_t *pthread_ids; // 薄脆线程池中线程id

    struct job *head;
    struct job *tail;  // 任务队列的尾
    int queue_max_num; // 任务队列的最多放多少个
    int queue_cur_num; // 任务队列已有多少个任务

    pthread_mutex_t mutex;
    pthread_cond_t queue_empty;     // 任务队列为空
    pthread_cond_t queue_not_emtpy; // 任务队列不为空
    pthread_cond_t queue_not_full;  // 任务队列不为满

    int pool_close; // 线程退出标志
};

void *threadpool_function(void *arg); // 任务队列取数据 执行线程函数

struct threadpool *threadpool_init(int thread_num, int queue_max_num);

void threadpool_add_job(struct threadpool *pool, void *(*func)(void *), void *arg); // 增加任务

void thread_destroy(struct threadpool *pool);

#endif