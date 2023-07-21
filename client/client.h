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
#include "../head/protocol.h"

//聊天室功能能选择界面
void menu();
//写线程 
void *write_thread(void * arg);
//读线程 
void *read_thread(void * arg);
//修改退出聊天室的方式
void Close(int signum);

//文件处理
void file_from(int sockfd);
void file_recv(char buffer[]);


#endif