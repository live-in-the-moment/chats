#include "client.h"
#include "../head/crc.h"

// 时间
time_t times;
struct tm *local_time;
char msg_time[1024];

// 修改退出聊天软件的方式
void Close(int signum)
{
    printf("请正确退出\n");
}

//序列化并发送数据
int send_data(Message *message){
    char buf[1024];
    char res[1024];
    message->header.crc32 = calculate_crc32(message);
    memcpy(&message, &buf, sizeof(message));
    send(sockfd, &message, sizeof(message), 0);
    return recv(sockfd, &res, sizeof(res), 0);
}

//重发
int retry(Message *message){
    for (size_t i = 0; i < 3; i++)
    {
        if (send_data(message) != -1){
            return 0;
        }
        printf("发送失败，正在进行第%ld次重发", i+1);
    }
    return -1;
}

// 聊天室 功能选择界面
void menu()
{
    printf("***********聊*************天*************室**************\n\n");
    printf("                          1.登录                       \n\n");
    printf("                          2.注册                       \n\n");
    printf("                          3.私聊                       \n\n");
    printf("                          4.群聊                       \n\n");
    printf("                          5.忘记密码                    \n\n");
    printf("                          6.查看在线用户                 \n\n");
    printf("                          7.查看聊天记录                 \n\n");
    printf("                          8.文件传输                  \n\n");
    printf("                          9.退出聊天室                 \n\n");
    printf("*********************************************************\n");
}

// 读线程
void *read_thread(void *arg)
{
    char receive[128];
    int length;

    struct Message node;
    node = *((struct Message *)arg);
    sockfd = *node.header.receiver;

    while (1)
    {
        memset(receive, 0, sizeof(receive));
        length = recv(sockfd, receive, 100, 0);
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
    char res[1024];
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
            scanf("%s", message.header.sender);
            printf("密码:\n");
            scanf("%s", message.body.login_request.password);
            strcpy(message.header.message_type, "LOGIN");
            printf("正在登录，请稍后......\n");
            sleep(1);
            
            if(retry(&message) == -1){
                printf("发送失败，请重新操作");
            }
            
            sleep(1);
            system("clear");
            break;
        // 注册
        case 2:
            printf("id:");
            scanf("%ls", message.header.sid);

            // printf("昵称:\n");
            // scanf("%s", message);

            printf("password:");
            scanf("%s", message.body.login_request.password);

            // printf("请输入密保:\n");
            // scanf("%s", message.secret);

            strcpy(message.header.message_type, "REG");
            printf("正在注册，请稍后......\n");
            sleep(1);

            if(retry(&message) == -1){
                printf("发送失败，请重新操作");
            }

            sleep(1);
            system("clear");
            break;
        // 私聊
        case 3:
            printf("请输入消息:\n");
            memset(&message, 0, sizeof(message));
            // fgets(sendline, 128, stdin);
            scanf("%s", message.body.chat_message.content);
            strcpy(message.header.message_type, "PRIVATE");
            printf("请输入对方的id:\n");
            scanf("%ls", message.header.rid);
            // 获取当前时间
            time(&times);
            // 将时间转换为本地时间
            local_time = localtime(&times);
            // 格式化时间字符串
            strftime(message.header.msg_time, sizeof(msg_time), "%Y-%m-%d %H:%M:%S", local_time);
            // strcpy(message.header.msg_time, msg_time);
            // send(sockfd, &message, sizeof(message), 0);

            if(retry(&message) == -1 ){
                printf("发送失败，请重试");
            }
            break;

        // 群聊
        case 4:
            while (1)
            {
                memset(&message, 0, sizeof(message));
                printf("请输入消息:\n");
                // memset(sendline, 0, sizeof(sendline));
                // fgets(sendline, sizeof(sendline), stdin);
                scanf("%s", message.body.chat_message.content);
                
                // 获取当前时间
                time(&times);
                // 将时间转换为本地时间
                local_time = localtime(&times);
                // 格式化时间字符串
                strftime(message.header.msg_time, sizeof(message.header.msg_time), "%Y-%m-%d %H:%M:%S", local_time);
                strcpy(message.header.message_type, "GROUP");
                // strcpy(message.header.msg_time, msg_time);
                // send(sockfd, &message, sizeof(message), 0);

                if(retry(&message) == -1){
                    printf("发送失败，请重试");
                    continue;
                }

                if (strcmp(message.body.chat_message.content,"quit") == 0)
                {
                    printf("已退出");
                    break;
                }
            }
            break;
        // // 忘记密码
        // case 5:
        //     printf("请输入id:\n");
        //     scanf("%s", message.id);
        //     getchar();
        //     printf("请输入密保:\n");
        //     scanf("%s", message.secret);
        //     getchar();
        //     printf("请输入新密码:\n");
        //     scanf("%s", message.passwd);
        //     getchar();
        //     strcpy(message.cmd, "FORGET");
        //     printf("正在更改密码，请稍后......\n");
        //     sleep(1);
        //     send(sockfd, &message, sizeof(message), 0);
        //     sleep(1);
        //     system("clear");
        //     break;
        // 查看在线人数
        case 6:
            strcpy(message.header.message_type, "LOOKUSERS");

            if(retry(&message) == -1){
                printf("查询失败，请重试");
            }
            // send(sockfd, &message, sizeof(message), 0);
            sleep(1);
            break;
        // 查看聊天记录
        case 7:
            strcpy(message.header.message_type, "LOOKCHATRECORD");
            if(retry(&message) == -1){
                printf("查询失败，请重试");
            }
            // send(sockfd, &message, sizeof(message), 0);
            break;
        // 文件传输
        case 8:
            // file_from(sockfd);
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
    strcpy(msg_text.header.message_type, "FILE");

    printf("请输入传输对象的id:\n");
    scanf("%ls", msg_text.header.rid);

    printf("请输入文件路径\n");
    scanf("%s", filename_path);

    if ((from_fd = open(filename_path, O_RDONLY)) == -1)
    {
        perror("open error!\n");
        printf("没发现此文件\n");
        exit(1);
    }

    // while (1)
    // {
    //     //************

    //     memset(msg_text.buffer, 0, sizeof(msg_text.buffer));
    //     // printf("buffer = %s\n",msg_text.buffer);
    //     bytes_read = read(from_fd, msg_text.buffer, 1024);

    //     if ((bytes_read == -1))
    //     {
    //         perror("read error!\n");
    //         exit(1);
    //     }
    //     if (bytes_read == 0)
    //     {
    //         break;
    //     }

    //     send(sockfd, &msg_text, sizeof(msg_text), 0);
    //     sleep(3);
    // }

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
