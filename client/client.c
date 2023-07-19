#include "client.h"

// 时间
time_t times;
struct tm *local_time;
char msg_time[1024];
int sockfd;

char mysid[32];
char chat_status[16] = {"group_chat"};
char private_list[8][32];
int private_items = sizeof(private_list) / sizeof(private_list[0]);

// 修改退出聊天软件的方式
void Close(int signum)
{
    printf("请正确退出\n");
}

void private_chats(char *sendline, Message m)
{
    Message message;
    strcpy(message.header.sid, m.header.sid);
    strcpy(message.header.rid, m.header.rid);
    strcpy(message.header.msg_type, "PRIVATE");

    while (1)
    {
        printf("请输入消息:\n");
        memset(sendline, 0, sizeof(sendline));
        // fgets(sendline, 128, stdin);
        scanf("%s", sendline);

        if (strncmp(sendline, "quit", 4) == 0)
        {
            printf("退出私聊");
            break;
        }

        // 获取当前时间
        time(&times);
        // 将时间转换为本地时间
        local_time = localtime(&times);
        // 格式化时间字符串
        strftime(msg_time, sizeof(msg_time), "%Y-%m-%d %H:%M:%S", local_time);
        strcpy(message.header.sid, message.header.sid);
        strcpy(message.body.chat_message.content, sendline);
        strcpy(message.header.msg_time, msg_time);
        // message.body.private_chat_response.accepted = false;
        printf("%d, %s, %s\n", message.body.private_chat_response.accepted, message.body.chat_message.content, sendline);
        int s = send(sockfd, &message, sizeof(message), 0);
        printf("%d\n%ld\n", s, sizeof(message));
    }
}

// 聊天室 功能选择界面
void menu()
{
    printf("***********聊*************天*************室**************\n\n");
    printf("                          1.登录                       \n\n");
    printf("                          2.注册                       \n\n");
    printf("                          3.私聊                       \n\n");
    printf("                          4.群聊                       \n\n");
    printf("                          5.查看在线用户                 \n\n");
    printf("                          6.查看群聊天记录                \n\n");
    printf("                          7.查看私聊天记录                \n\n");
    printf("                          8.文件传输                  \n\n");
    printf("                          9.退出聊天室                 \n\n");
    printf("*********************************************************\n");
}

// 读线程
void *read_thread(void *arg)
{
    Message response;
    int length;

    struct Message node;
    node = *((struct Message *)arg);
    sockfd = node.header.cfd;

    while (1)
    {
        memset(&response, 0, sizeof(response));
        length = recv(sockfd, &response, sizeof(response), 0);
        printf("%d 读到服务端发送的包 %d ....\n",response.body.response.res_type, length);
        if (length == 0)
        {
            pthread_exit(NULL);
        }
        
        if (response.header.msg_type)
        {
            /* code */
        }
        
        switch (response.body.response.res_type)
        {
        case 1:
            strcpy(mysid, response.header.sid);
            printf("%s，欢迎%s\n", response.body.response.logs, mysid);
            break;
        case 2:
            strcpy(chat_status, "private_true");
            printf("对方接受私聊申请\n");
            break;
        case 3:
            char private_item[32];
            sprintf(private_item, "%s (%s)\n", response.header.rid, response.header.msg_time);
            printf("%s %s 请在主菜单输入 3 查看\n", private_item, response.body.response.logs);
            strcpy(private_list[private_items], private_item);
            private_items ++;
            break;
        case 4:
            printf("%s", response.body.response.logs);
            break;
        case 5:
            printf("%s\t%s\n", response.body.response.logs, response.header.sid);
            break;
        case 6:
            printf("%s", response.body.response.logs);
            break;
        case 7:
            printf("%s", response.body.response.logs);
            break;
        case 8:
            printf("%s", response.body.response.logs);
            printf("接收文件中....\n");
            char buffer[1024];
            memset(buffer, 0, sizeof(buffer));
            int file_len = recv(sockfd, buffer, 1024, 0);
            if (-1 == file_len)
            {
                perror("recv");
                exit(-1);
            }
            printf("file_len = %d\n", file_len);
            buffer[file_len] = '\0';
            file_recv(buffer);
            printf("接收文件成功\n");
            break;
        case 9:
            printf("%s", response.body.response.logs);
            break;
        case 0:
            printf("%s\n", response.body.response.logs);
            break;
        }

        // else if (strstr(receive, "请求与你私聊") != NULL)
        // {
        //     char acc[8];
        //     printf("%s 请求私聊，是否同意【Y/n】\n", strtok(receive, ","));
        //     // scanf("%s", acc);
        //     // if (strcasecmp(acc, "Y") == 0)
        //     // {
        //     //     send(sockfd, &acc, sizeof(acc), 0);
        //     // }

        // }
        // else
        // {
        //     printf("%s\n", response.body.response.logs);
        // }
    }
    pthread_exit(NULL);
}

// 写线程
void *write_thread(void *arg)
{

    char sendline[1024];
    Message message;
    int select;
    sockfd = *((int *)arg);

    while (1)
    {
        // system("clear");
        menu();
        memset(&select, 0, sizeof(int));
        scanf("%d", &select);
        getchar();

        switch (select)
        {
        // 登录
        case 1:
            printf("账号:\n");
            scanf("%s", message.header.sid);
            getchar();
            strcpy(message.header.rid, message.header.sid);
            printf("密码:\n");
            scanf("%s", message.body.login_request.password);
            getchar();
            strcpy(message.header.msg_type, "LOGIN");
            printf("正在登录，请稍后......\n");
            sleep(1);
            send(sockfd, &message, sizeof(message), 0);

            sleep(2);
            system("clear");
            break;
        // 注册
        case 2:
            printf("id:\n");
            scanf("%s", message.header.sid);
            getchar();
            printf("密码:\n");
            scanf("%s", message.body.login_request.password);
            getchar();
            strcpy(message.header.msg_type, "REG");
            printf("正在注册，请稍后......\n");
            sleep(1);
            send(sockfd, &message, sizeof(message), 0);
            sleep(2);
            system("clear");
            break;
        // 私聊
        case 3:
            printf("1、查看私聊申请。\n2、发起私聊会话。\n");
            int op;
            scanf("%d", &op);
            switch (op)
            {
            case 1:
                char user[32];
                printf("************************\n");
                for (int i = 0; i < private_items; i++) {
                    printf("%s\n", private_list[i]);
                }
                printf("************************\n");
                printf("请输入对应的用户名接受私聊，quit退出\n");
                scanf("%s", user);
                if (strncmp(user, "quit", 4) == 0)
                {
                    break;
                }
                Message req;
                strcpy(req.header.sid, mysid);
                strcpy(req.header.msg_type, "PRIVATE");
                strcpy(message.header.chat_status, "private_accept");
                strcpy(req.header.rid, user);
                strcpy(chat_status, "private_accept");
                send(sockfd, &req, sizeof(req), 0);

                private_chats(sendline, req);
                printf("私聊结束\n");
                break;

            case 2:
                strcpy(message.header.msg_type, "PRIVATE");
                printf("请输入对方的Id:\n");
                scanf("%s", message.header.rid);

                message.body.private_chat_response.accepted = 0;
                strcpy(message.header.chat_status, "private_true");
                send(sockfd, &message, sizeof(message), 0);
                printf("%s\n", "等待对方响应...\n");

                time_t start_time = time(NULL);
                while (1)
                {
                    int timeout = 5;
                    time_t current_time = time(NULL);
                    if (strcmp(chat_status, "private_true") == 0)
                    {
                        private_chats(sendline, message);
                        break;
                    }
                    if (current_time - start_time > timeout)
                    {
                        printf("对方未理会你\n");
                        
                        strcpy(message.header.chat_status, "group_chat");
                        strcpy(chat_status, "private_true");
                        send(sockfd, &message, sizeof(message), 0);
                        printf("%s\n", chat_status);
                        break;
                    }
                    sleep(1);
                }
                // private_chats(sendline, message);
                break;
            }
            break;

        // 群聊
        case 4:
            while (1)
            {
                printf("请输入消息:\n");
                memset(sendline, 0, sizeof(sendline));
                scanf("%s", sendline);
                // fgets(sendline, sizeof(sendline), stdin);
                if (strncmp(sendline, "quit", 4) == 0)
                {
                    printf("已退出群聊模式");
                    break;
                }
                // 获取当前时间
                time(&times);
                // 将时间转换为本地时间
                local_time = localtime(&times);
                // 格式化时间字符串
                strftime(msg_time, sizeof(msg_time), "%Y-%m-%d %H:%M:%S", local_time);
                strcpy(message.header.sid, message.header.sid);
                strcpy(message.header.msg_type, "GROUP");
                strcpy(message.body.chat_message.content, sendline);
                strcpy(message.header.msg_time, msg_time);
                send(sockfd, &message, sizeof(message), 0);
            }
            break;
        // 查看在线人数
        case 5:
            strcpy(message.header.msg_type, "LOOKUSERS");
            send(sockfd, &message, sizeof(message), 0);
            sleep(1);
            break;
        // 查看聊天记录
        case 6:
            strcpy(message.header.msg_type, "LOOKCHATRECORD");
            send(sockfd, &message, sizeof(message), 0);
            break;
        // 查询私聊记录
        case 7:
            printf("请输入需要查询记录的ID:\n");
            scanf("%s", message.header.sid);
            strcpy(message.header.msg_type, "LOOKPMCHATRECORD");
            send(sockfd, &message, sizeof(message), 0);
            sleep(3);
            break;
        // 文件传输
        case 8:
            file_from(sockfd);
            break;
        // 退出聊天室
        case 9:
            system("clear");
            // sleep(1);
            printf("退出成功!\n");
            printf("欢迎下次使用！\n");
            exit(-1);
            break;
        default:
            printf("选择有误！请重新输入！\n");
            break;
        }
    }
}

void file_from(int sockfd)
{

    char sendline[100];
    struct Message msg_text;

    int from_fd;
    int bytes_read;
    char *from_ptr;
    char filename_path[1024];
    strcpy(msg_text.header.msg_type, "FILE");

    printf("请输入传输对象的id:\n");
    scanf("%s", msg_text.header.sid);

    printf("请输入文件路径\n");
    scanf("%s", filename_path);

    if ((from_fd = open(filename_path, O_RDONLY)) == -1)
    {
        perror("open error!\n");
        printf("没发现此文件\n");
        exit(1);
    }

    while (1)
    {

        memset(msg_text.body.file_transfer.file_path, 0, sizeof(msg_text.body.file_transfer.file_path));
        bytes_read = read(from_fd, msg_text.body.file_transfer.file_path, 1024);

        if ((bytes_read == -1))
        {
            perror("read error!\n");
            exit(1);
        }
        if (bytes_read == 0)
        {
            break;
        }

        send(sockfd, &msg_text, sizeof(msg_text), 0);
        sleep(3);
    }

    close(from_fd);
}

void file_recv(char buffer[])
{
    int to_fd;
    char filename_path[1024];
    char *to_ptr;
    int bytes_write;

    if ((to_fd = open("1.txt", O_APPEND | O_CREAT | O_WRONLY, 0655)) == -1)
    {
        perror("open error!\n");
        exit(1);
    }

    to_ptr = buffer;

    bytes_write = write(to_fd, to_ptr, strlen(buffer));

    if ((bytes_write == -1))
    {
        perror("write error!\n");
        exit(1);
    }

    close(to_fd);
}
