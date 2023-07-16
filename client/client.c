#include "client.h"


// 时间
time_t times;
struct tm *local_time;
char msg_time[1024];
int sockfd;

// 修改退出聊天软件的方式
void Close(int signum)
{
    printf("请正确退出\n");
}

void private_chats(char* sendline, Message m){
    Message message;
    // memset(&message, 0, sizeof(message));
    strcpy(message.header.sid, m.header.sid);
    strcpy(message.header.rid, m.header.rid);
    strcpy(message.header.msg_type, "PRIVATE");
    // message.body.private_chat_response.accepted = 1;
    // while (1)
    // {
    //     char res[16];
    //     if (recv(sockfd, res, sizeof(res), 0))
    //     {
    //         /* code */
    //     }
        
    // }
    // char res[16];
    // recv(sockfd, res, sizeof(res), 0);
    // if (strcasecmp(res, "Y") != 0)
    // {
    //     printf("对方拒绝了你的私聊申请！");
    //     return;
    // }
    
    
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

//  序列化并发送数据
// int send_data(int sockfd,Message *message)
// {
//     char buf[1024];
//     // char res[1024];
//     // message->header.crc32 = calculate_crc32(message);
//     memcpy(&message, &buf, sizeof(message));
//     send(sockfd, &buf, sizeof(message), 0);
//     return 0;
// }

//  重发
// int retry(int sockfd,Message *message)
// {
//     for (size_t i = 0; i < 3; i++)
//     {
//         if (send_data(sockfd,message) != -1)
//         {
//             return 0;
//         }
//         printf("发送失败，正在进行第%ld次重发", i + 1);
//         return -1;
//     }

// }

// 聊天室 功能选择界面
void menu()
{
    printf("***********聊*************天*************室**************\n\n");
    printf("                          1.登录                       \n\n");
    printf("                          2.注册                       \n\n");
    printf("                          3.私聊                       \n\n");
    printf("                          4.群聊                       \n\n");
    printf("                          5.查看在线用户                 \n\n");
    printf("                          6.查看聊天记录                 \n\n");
    printf("                          7.文件传输                  \n\n");
    printf("                          8.退出聊天室                 \n\n");
    printf("*********************************************************\n");
}

// 读线程
void *read_thread(void *arg)
{
    char receive[128];
    int length;

    struct Message node;
    node = *((struct Message *)arg);
    sockfd = node.header.cfd;

    while (1)
    {
        memset(receive, 0, sizeof(receive));
        length = recv(sockfd, receive, 100, 0);
        printf("读到客户端发送的包....\n");
        if (length == 0)
        {
            pthread_exit(NULL);
        }

        receive[length] = '\0';
        if (strcmp("AAAAA", receive) == 0)
        {
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
            // printf("buffer = %s\n",buffer);
            file_recv(buffer);
            printf("接收文件成功\n");
        }
        else if (strstr(receive, "请求与你私聊") != NULL)
        {
            char acc[8];
            printf("%s 请求私聊，是否同意【Y/n】\n", strtok(receive, ","));
            // scanf("%s", acc);
            // if (strcasecmp(acc, "Y") == 0)
            // {
            //     send(sockfd, &acc, sizeof(acc), 0);
            // }
            
        }
        else
        {
            printf("%s\n", receive);
        }
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
        system("clear");
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
            strcpy(message.header.msg_type, "PRIVATE");
            printf("请输入对方的Id:\n");
            scanf("%s", message.header.rid);

            message.body.private_chat_response.accepted = 0;
            send(sockfd, &message, sizeof(message), 0);
            printf("%d\n", message.body.private_chat_response.accepted);

            private_chats(sendline, message);
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
            sleep(2);
            break;
        // 查看聊天记录
        case 6:
            strcpy(message.header.msg_type, "LOOKCHATRECORD");
            send(sockfd, &message, sizeof(message), 0);
            break;
        // 文件传输
        case 7:
            file_from(sockfd);
            break;
        // 退出聊天室
        case 8:
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
        //************

        memset(msg_text.body.file_transfer.file_path, 0, sizeof(msg_text.body.file_transfer.file_path));
        // printf("buffer = %s\n",msg_text.buffer);
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
