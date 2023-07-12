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
#include<signal.h>
#include <stdbool.h>

// 客户端套接字 socket
int sockfd;

// 功能指令
enum {
    REG,    //注册
    LOGIN,  //登录
    FORGET, //忘记密码
    LOOKUSERS,  //查看在线用户
    PRIVATE,    //私聊
    GROUP,  //群聊
    LOOKCHATRECORD, //查看聊天记录
    // FILE,   // 传输文件
};

// 保存信息的结构体
// typedef struct Message{
//     char id[32];    //账号
//     char myid[32];  //用于保存自己的ID
//     char name[32];  //名称
//     char passwd[32];    //密码
//     char secret[32];    //密保
//     char cmd[32];   //聊天方式
//     int cfd;    //聊天对象
//     char msg[1024]; //聊天内容
//     char buffer[1024]; // 文件处理
//     char msg_time[1024]; //信息产生时间
// }Message;

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