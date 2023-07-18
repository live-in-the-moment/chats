#include "server.h"

// 创建一个空链表(保存在线用户)
void CreateLink(OnlineLinkList **head)
{
    CreateNode(head);
    (*head)->next = NULL; // 初始化头指针指向空
}
// 创建新的结点
void CreateNode(OnlineLinkList **new_node)
{
    // 定义新的结点，在堆区开辟空间
    *new_node = (OnlineLinkList *)malloc(sizeof(OnlineLinkList));
    // 判断新结点开辟新空间是否成功
    if (NULL == new_node)
    {
        printf("malloc error!\n"); // 打印错误信息
        exit(-1);                  // 异常退出
    }
}
// 头插法插入数据
void InsertNodeHead(OnlineLinkList *head, OnlineLinkList *new_node)
{
    // 新的结点指向第一个结点的地址
    new_node->next = head->next;
    // 头结点指向第一个结点
    head->next = new_node;
}

// 客户端下线将cfd从链表中删除
void DeleteNode(thread_node *node)
{
    OnlineLinkList *p1 = NULL;
    OnlineLinkList *p2 = NULL;
    OnlineLinkList *head = NULL;
    int cfd;

    head = node->head;
    cfd = node->cfd;

    p1 = head->next;
    p2 = head;

    if (p1 == NULL)
    {
    }
    else
    {
        // cfd判断下线
        while (p1 != NULL && p1->cfd != cfd)
        {
            p2 = p1;
            p1 = p1->next;
        }

        if (p1 == NULL)
        {
            printf("no such Message!\n");
        }
        else
        {
            p2->next = p1->next;
            free(p1);
        }
    }
}

// 线程函数
void *MyFun(void *arg)
{
    thread_node node;

    node = *((struct thread_node *)arg);

    while (1)
    {
        // 收发消息
        MsgSendRecv(&node);
    }

    // 关闭当前通信接口
    close(node.cfd);

    return NULL;
}

// 查找id是否存在
int FindId(sqlite3 *ppdb, Message *data)
{
    char sq1[128] = {0};
    sprintf(sq1, "select *from mytable;");
    char **result;
    int row, column;
    int flag = 0;

    int ret = sqlite3_get_table(ppdb, sq1, &result, &row, &column, NULL);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_get_table: %s\n", sqlite3_errmsg(ppdb));
        return 1;
    }
    int Index = column;
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < column; j++)
        {
            if (Index % column == 0)
            {
                if (strcmp(result[Index], data->header.sid) == 0)
                {
                    flag = 1;
                    break;
                }
            }
            Index++;
        }
        if (flag == 1)
        {
            break;
        }
    }
    if (flag == 1)
    {
        sqlite3_free_table(result);
        return -1;
    }
    sqlite3_free_table(result);
}

// 注册
void Register(thread_node *node, Message *data)
{
    // 记录cfd 传回注册成功
    data->header.cfd = node->cfd;
    // 注册前查找是否被注册
    if (-1 == FindId(node->ppdb, data))
    {
        Message register_res;
        register_res.body.response.res_type = 0;
        strcpy(register_res.body.response.logs, "账号已经存在,请重新注册");
        send(data->header.cfd, &register_res, sizeof(register_res), 0);
        return;
    }
    else
    {
        // 像数据库中插入数据
        InsertData(node->ppdb, data);
    }
}

// 向数据库中插入数据
void InsertData(sqlite3 *ppdb, Message *data)
{
    char str[128];
    char *sql = str;
    char *errmsg = NULL;

    sprintf(sql, "insert into mytable(sid,password) values('%s','%s');",
            data->header.sid, data->body.login_request.password);

    if (SQLITE_OK != sqlite3_exec(ppdb, sql, NULL, NULL, &errmsg))
    {
        printf("insert record fail! %s \n", errmsg);
        sqlite3_close(ppdb);
        exit(-1);
    }

    Message register_res;
    register_res.body.response.res_type = 0;
    strcpy(register_res.body.response.logs, "账号注册成功");
    send(data->header.cfd, &register_res, sizeof(register_res), 0);
}

// 创建表(用于保存注册用户的信息)
void CreatTable(sqlite3 *ppdb)
{
    // 创建表
    char sq1[128] = {0};
    sprintf(sq1, "create table if not exists mytable(sid char,password char);");
    int ret = sqlite3_exec(ppdb, sq1, NULL, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec: %s\n", sqlite3_errmsg(ppdb));
        exit(-1);
    }
}


// 查找id和密保是否存在
int FindSecret(sqlite3 *ppdb, Message *data)
{
    char sq1[128] = {0};
    sprintf(sq1, "select *from mytable;");
    char **result;
    int row, column;
    int flag = 0;
    int ret = sqlite3_get_table(ppdb, sq1, &result, &row, &column, NULL);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_get_table: %s\n", sqlite3_errmsg(ppdb));
        return -1;
    }
    int Index = column;
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < column; j++)
        {
            if (Index % column == 0)
            {
                int ret1 = strcmp(result[Index], data->header.sid);
                if (ret1 == 0)
                {
                    flag = 1;
                    break;
                }
            }
            Index++;
        }
        if (flag == 1)
        {
            break;
        }
    }
    if (flag == 1)
    {
        sqlite3_free_table(result);
        return -1;
    }
    sqlite3_free_table(result);
}

// 更新数据库(更改密码)
void UpdateData(sqlite3 *ppdb, Message *data)
{
    char sq1[128] = {0};
    sprintf(sq1, "update mytable set passwd='%s' where id= '%s'  ;", data->body.login_request.password, data->header.sid);
    char **result;
    int row, column;
    int flag = 0;

    int ret = sqlite3_get_table(ppdb, sq1, &result, &row, &column, NULL);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_get_table: %s\n", sqlite3_errmsg(ppdb));
        exit(-1);
    }

    Message res;
    res.body.response.res_type = 0;
    strcpy(res.body.response.logs, "密码更改成功");
    send(data->header.cfd, &res, sizeof(res), 0);

    // char res[100] = {"密码更改成功"};
    // send(data->header.cfd, res, strlen(res), 0);
}

// 检查账号是否重复登录
int RepeatLogin(thread_node *node, Message *data)
{
    OnlineLinkList *head = NULL;
    head = node->head;
    OnlineLinkList *p = NULL;
    p = head->next;

    if (p == NULL)
    {
        // 无用户在线
        return 0;
    }

    while (p != NULL && strcmp(p->id, data->header.sid) != 0)
    {
        p = p->next;
    }

    if (p == NULL)
    {
        // 该id不在线
        return 0;
    }
    else
    {
        // 已经登录
        return -1;
    }
}

// 登录
void Login(thread_node *node, Message *data)
{
    if (-1 == RepeatLogin(node, data))
    {
        Message res;
        res.body.response.res_type = 0;
        strcpy(res.body.response.logs, "您已经在线，无需重复登录");
        send(node->cfd, &res, sizeof(res), 0);

        // char res[128] = {"您已经在线，无需重复登录"};
        // send(node->cfd, res, strlen(res), 0);
        return;
    }

    OnlineLinkList *new_node;
    OnlineLinkList *head = node->head;
    // 记录cfd 传回成功
    data->header.cfd = node->cfd;

    if (-1 != FindId(node->ppdb, data))
    {
        Message res;
        res.body.response.res_type = 0;
        strcpy(res.body.response.logs, "该账号不存在，请重新登录");
        send(data->header.cfd, &res, sizeof(res), 0);

        // char res[128] = {"该账号不存在，请重新登录"};
        // send(data->header.cfd, res, strlen(res), 0);
        return;
    }
    else
    {
        // 登录前查找账号跟密码是否正确
        int ret = VerifyIdPassword(node->ppdb, data);
        if (ret == -1)
        {
            Message res;
            res.body.response.res_type = 0;
            strcpy(res.body.response.logs, "登录成功");
            send(data->header.cfd, &res, sizeof(res), 0);

            // char res[128] = {"登录成功"};
            // send(data->header.cfd, res, strlen(res), 0);

            // 创建新的节点
            CreateNode(&new_node);

            // 把该账户的id复制的链表里
            strcpy(new_node->id, data->header.sid);

            new_node->cfd = data->header.cfd;
            new_node->forbid_flag = 0;

            // 头插法插入新的数据
            InsertNodeHead(head, new_node);
        }
        else
        {
            Message res;
            res.body.response.res_type = 0;
            strcpy(res.body.response.logs, "账号或密码错误");
            send(data->header.cfd, &res, sizeof(res), 0);

            // char res[128] = {"账号或密码错误"};
            // send(data->header.cfd, res, strlen(res), 0);
        }
    }
}

// 验证登录的账号和密码
int VerifyIdPassword(sqlite3 *ppdb, Message *data)
{
    char sq1[128] = {0};
    sprintf(sq1, "select *from mytable;");
    char **result;
    int row, column;
    int flag = 0;

    int ret = sqlite3_get_table(ppdb, sq1, &result, &row, &column, NULL);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_get_table: %s\n", sqlite3_errmsg(ppdb));
        return -1;
    }
    int Index = column;
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < column; j++)
        {
            if (Index % column == 0)
            {

                int ret1 = strcmp(result[Index], data->header.sid);
                int ret2 = strcmp(result[Index + 1], data->body.login_request.password);

                if (ret1 == 0 && ret2 == 0)
                {
                    flag = 1;
                    break;
                }
            }
            Index++;
        }
        if (flag == 1)
        {
            break;
        }
    }

    if (flag == 1)
    {
        sqlite3_free_table(result);
        return -1;
    }
    sqlite3_free_table(result);
}

// 收发消息
void MsgSendRecv(thread_node *node)
{
    int ret;

    char buf[1024];


    Message RecvInfo;

    ret = recv(node->cfd, &RecvInfo, sizeof(RecvInfo), 0);

    // memcpy(buf,&RecvInfo,sizeof(RecvInfo));

    if (ret == 0)
    {
        DeleteNode(node);
        pthread_exit(NULL);
    }
    else
    {
        // 注册
        if (strcmp(RecvInfo.header.msg_type, "REG") == 0)
        {
            Register(node, &RecvInfo);
        }
        // 登录
        else if (strcmp(RecvInfo.header.msg_type, "LOGIN") == 0)
        {
            Login(node, &RecvInfo);
        }
        // 查看在线用户
        else if (strcmp(RecvInfo.header.msg_type, "LOOKUSERS") == 0)
        {

            if (-1 == InspectOwnOnline(node))
            {
                Message res;
                res.body.response.res_type = 0;
                strcpy(res.body.response.logs, "你未在线，不能查看在线用户，请先登录");
                send(node->cfd, &res, sizeof(res), 0);

                // char res[128] = {"你未在线，不能查看在线用户，请先登录"};
                // send(node->cfd, res, strlen(res), 0);
            }
            else
            {
                LookOnlineUsers(node);
            }
        }
        // 群聊
        else if (strcmp(RecvInfo.header.msg_type, "GROUP") == 0)
        {
            if (-1 == InspectOwnOnline(node))
            {
                Message res;
                res.body.response.res_type = 0;
                strcpy(res.body.response.logs, "你未在线，不能群发消息，请先登录");
                send(node->cfd, &res, sizeof(res), 0);

                // char res[128] = {"你未在线，不能群发消息，请先登录"};
                // send(node->cfd, res, strlen(res), 0);
            }
            else
            {
                GroupChat(node->ppdb, node->head, &RecvInfo);
            }
        }
        // 私聊
        else if (strcmp(RecvInfo.header.msg_type, "PRIVATE") == 0)
        {
            if (-1 == InspectOwnOnline(node))
            {
                Message res;
                res.body.response.res_type = 0;
                strcpy(res.body.response.logs, "你未在线，不能私发消息，请先登录");
                send(node->cfd, &res, sizeof(res), 0);

                // char res[128] = {"你未在线，不能私发消息，请先登录"};
                // send(node->cfd, res, strlen(res), 0);
            }
            // else if (RecvInfo.body.private_chat_response.accepted)
            // {
            //     char res[128] = {"发送私聊请求成功"};
            //     send(node->cfd, res, strlen(res), 0);
            // }
            else
            {
                PrivateChat(node, &RecvInfo);
            }
        }

        // 查看聊天记录
        else if (strcmp(RecvInfo.header.msg_type, "LOOKCHATRECORD") == 0)
        {

            if (-1 == InspectOwnOnline(node))
            {
                Message res;
                res.body.response.res_type = 0;
                strcpy(res.body.response.logs, "你未在线，不能查看聊天记录，请先登录");
                send(node->cfd, &res, sizeof(res), 0);

                // char res[128] = {"你未在线，不能查看聊天记录，请先登录"};
                // send(node->cfd, res, strlen(res), 0);
            }
            else
                PrintChatRecord(node->ppdb, node);
        }
        // 传输文件
        else if (strcmp(RecvInfo.header.msg_type, "FILE") == 0)
        {

            if (-1 == InspectOwnOnline(node))
            {
                Message res;
                res.body.response.res_type = 0;
                strcpy(res.body.response.logs, "你未在线，不能传输文件，请先登录");
                send(node->cfd, &res, sizeof(res), 0);

                // char res[128] = {"你未在线，不能传输文件，请先登录"};
                // send(node->cfd, res, strlen(res), 0);
            }
            else
                FileRecv(node, &RecvInfo);
        }
    }
}

void FileRecv(thread_node *node, Message *data)
{
    if (-1 == InspectOwnOnline(node))
    {
        Message res;
        res.body.response.res_type = 0;
        strcpy(res.body.response.logs, "你未在线，不能发文件");
        send(node->cfd, &res, sizeof(res), 0);

        // char res[100] = {"你未在线，不能发文件"};
        // send(node->cfd, res, strlen(res), 0);
        return;
    }

    int len;

    OnlineLinkList *p = NULL;

    p = node->head->next;

    // 寻找在线用户链表中的cfd与私聊的cfd是否一致
    while (p != NULL && strcmp(p->id, data->header.sid) != 0)
    {
        // printf("****%s\n",p->id);
        p = p->next;
    }

    // 判断是否在线
    if (p == NULL)
    {
        Message res;
        res.body.response.res_type = 0;
        strcpy(res.body.response.logs, "该用户不在线");
        send(node->cfd, &res, sizeof(res), 0);

        // printf("client is not online!\n");
        // char res[100] = {"该用户不在线"};
        // send(node->cfd, res, strlen(res), 0);
    }
    else
    {
        printf("正在接收中......\n");

        char arr[100] = {"AAAAA"};
        send(p->cfd, arr, strlen(arr), 0);
        usleep(30);
        // 向客户端发送文件
        send(p->cfd, data->body.file_transfer.file_path, strlen(data->body.file_transfer.file_path), 0);

    }
}

// 遍历用户注册信息
void Id(sqlite3 *ppdb)
{
    char sq1[128] = {0};
    sprintf(sq1, "select *from mytable;");
    char **result;
    int row, column;
    int flag = 0;

    int ret = sqlite3_get_table(ppdb, sq1, &result, &row, &column, NULL);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_get_table: %s\n", sqlite3_errmsg(ppdb));
        exit(-1);
    }
    int Index = column;
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < column; j++)
        {
            printf("%s = %s  ", result[j], result[Index]);

            Index++;
        }
        putchar(10);
    }
}
// 群聊
void GroupChat(sqlite3 *ppdb, OnlineLinkList *head, Message *data)
{

    int length;
    OnlineLinkList *p = NULL;
    p = head->next;
    length = strlen(data->body.chat_message.content);
    int len;

    char chat[2048] = {0};
    strcpy(chat, data->header.sid);
    strcat(chat, "(");
    strcat(chat, data->header.msg_time);
    strcat(chat, ")");
    strcat(chat, ":");
    strcat(chat, data->body.chat_message.content);

    InsertChatData(ppdb, chat);

    while (p != NULL)
    {
        // 发给每一个客户端
        len = strlen(chat);
        send(p->cfd, chat, len, 0);
        p = p->next;
    }
}

// 私聊
void PrivateChat(thread_node *node, Message *data)
{
    int length;

    OnlineLinkList *p = NULL;

    p = node->head->next;

    length = strlen(data->body.chat_message.content);
    int len;
    // 寻找在线用户链表中的cfd与私聊中的cfd是否一致
    while (p != NULL && strcmp(p->id, data->header.rid) != 0)
    {
        p = p->next;
    }

    if (p == NULL)
    {
        Message res;
        res.body.response.res_type = 0;
        strcpy(res.body.response.logs, "该用户不在线");
        send(node->cfd, &res, sizeof(res), 0);

        // char res[100] = {"该用户不在线"};
        // send(node->cfd, res, strlen(res), 0);
    }
    else if (!data->body.private_chat_response.accepted)
    {
        // char private_list = {
        //     rid = data.sid ,
        //     req_time data.msg_time
        // };
        // data->header.sid private_list.
        Message res;
        time_t times;
        struct tm *local_time;
        res.body.response.res_type = 3;
        strcpy(res.header.rid, data->header.sid);
        strcpy(res.body.response.logs, "请求与你私聊");

        time(&times);
        local_time = localtime(&times);
        strftime(res.header.msg_time, sizeof(res.header.msg_time), "%Y-%m-%d %H:%M:%S", local_time);

        send(p->cfd, &res, sizeof(res), 0);

        // char res[100] = {0};
        // char acc[8];
        // sprintf(res, "%d,请求与你私聊", node->cfd);
        // send(p->cfd, res, strlen(res), 0);
        // recv(p->cfd, acc, strlen(acc), 0);

        // send(node->cfd, acc, strlen(acc), 0);

        
    }
    else
    {
        
        char chat[2048] = {0};
        strcpy(chat, data->header.sid);
        strcat(chat, "(");
        strcat(chat, data->header.msg_time);
        strcat(chat, ")");
        strcat(chat, ":");
        strcat(chat, data->body.chat_message.content);

        len = strlen(chat);
        send(p->cfd, chat, len, 0);
        InsertChatData(node->ppdb, chat);
    }
}

// 查看在线用户
int LookOnlineUsers(thread_node *node)
{
    OnlineLinkList *head = NULL;
    head = node->head;
    OnlineLinkList *p = NULL;
    p = head->next;

    char bb[128];

    if (p == NULL)
    {
        return 0; // 当前无用户在线
    }

    while (p != NULL)
    {
        memset(bb, 0, sizeof(bb));
        strcpy(bb, p->id);
        send(node->cfd, bb, strlen(bb), 0);
        p = p->next;
        usleep(4);
    }

    return 1;
}

// 检查自己是否在线
int InspectOwnOnline(thread_node *node)
{
    OnlineLinkList *head = NULL;
    head = node->head;
    OnlineLinkList *p = NULL;
    p = head->next;

    if (p == NULL)
    {
        // 无用户在线
        return -1;
    }

    while (p != NULL && p->cfd != node->cfd)
    {
        p = p->next;
    }

    if (p == NULL)
    {
        // 自己不在线
        return -1;
    }
    else
    {
        // 自己已经登录
        return 0;
    }
}

// 创建第二张表用于保存聊天记录
void CreatTable2(sqlite3 *ppdb)
{
    // 创建表
    char sql[128] = {0};
    sprintf(sql, "create table if not exists chat(chat char);");
    int ret = sqlite3_exec(ppdb, sql, NULL, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec: %s\n", sqlite3_errmsg(ppdb));
        exit(-1);
    }
}

// 向第二张表中插入聊天记录
void InsertChatData(sqlite3 *ppdb, char *chat)
{
    char str[2048];
    char *sql = str;
    char *errmsg = NULL;

    sprintf(sql, "insert into chat(chat) values('%s');", chat);

    if (SQLITE_OK != sqlite3_exec(ppdb, sql, NULL, NULL, &errmsg))
    {
        printf("insert record fail! %s \n", errmsg);
        sqlite3_close(ppdb);
        exit(-1);
    }

    sprintf(sql, "insert into chat(chat) values('%s');", "\n");
    if (SQLITE_OK != sqlite3_exec(ppdb, sql, NULL, NULL, &errmsg))
    {
        printf("insert record fail! %s \n", errmsg);
        sqlite3_close(ppdb);
        exit(-1);
    }
}

// 遍历聊天记录
void PrintChatRecord(sqlite3 *ppdb, thread_node *node)
{
    OnlineLinkList *head = NULL;
    head = node->head;
    OnlineLinkList *p = NULL;
    p = head->next;

    char sql[128] = {0};
    sprintf(sql, "select *from chat");
    char **result;
    int row, column;

    int ret = sqlite3_get_table(ppdb, sql, &result, &row, &column, NULL);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_get_table: %s\n", sqlite3_errmsg(ppdb));
        exit(-1);
    }

    int Index = column;
    char chat[1000] = {0};

    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < column; j++)
        {
            strcpy(chat, result[Index]);
            send(node->cfd, chat, strlen(chat), 0);
            Index++;
        }
    }
    if (row == 0) // 当聊天记录为0行的时候聊天记录为空
    {
        strcpy(chat, "当前还没有聊天记录");
        send(node->cfd, chat, strlen(chat), 0);
        return;
    }
    else
    {
        return;
    }
}

//***************************************************
// 线程池

void *threadpool_function(void *arg) // 任务队列取数据 执行任务
{

    struct threadpool *pool = (struct threadpool *)arg;
    struct job *pjob = NULL;

    while (1)
    {
        // 我访问时别人不能让问  访问任务队列10个一个一个来
        pthread_mutex_lock(&(pool->mutex));

        while (pool->queue_cur_num == 0)
        {
            // 当目前任务队列没有任务  等待任务队列不为空的条件被置位(添加任务处成功过来唤醒)
            pthread_cond_wait(&(pool->queue_not_emtpy), &(pool->mutex));

            // 线程要结束时 退出
            if (pool->pool_close == 1)
            {
                pthread_exit(NULL);
            }
        }

        pjob = pool->head;     // 将对头任务拿出去处理
        pool->queue_cur_num--; // 任务数量减一个

        if (pool->queue_cur_num != pool->queue_max_num)
        {
            // 如果任务不满   不满条件唤醒
            pthread_cond_broadcast(&(pool->queue_not_full));
        }

        if (pool->queue_cur_num == 0)
        {
            // 当前任务队列没有任务
            pool->head = pool->tail = NULL;
            // 当任务队列为空时 唤醒空条件，去销毁线程池
            pthread_cond_broadcast(&(pool->queue_empty));
        }
        else
        {
            pool->head = pjob->next; // 处理完一个 队头向后移动一个
        }

        pthread_mutex_unlock(&(pool->mutex));

        (*(pjob->func))(pjob->arg); // 让线程执行任务队列里的任务
        free(pjob);                 // 执行完释放
        pjob = NULL;
    }
}

struct threadpool *threadpool_init(int thread_num, int queue_max_num)
{
    struct threadpool *pool = (struct threadpool *)malloc(sizeof(struct threadpool));
    // malloc

    pool->queue_max_num = queue_max_num;
    pool->queue_cur_num = 0;
    pool->pool_close = 0; // 线程退出标志 0不退出
    pool->head = NULL;
    pool->tail = NULL;

    pthread_mutex_init(&(pool->mutex), NULL);
    pthread_cond_init(&(pool->queue_empty), NULL);
    pthread_cond_init(&(pool->queue_not_emtpy), NULL);
    pthread_cond_init(&(pool->queue_not_full), NULL);

    pool->thread_num = thread_num;
    pool->pthread_ids = (pthread_t *)malloc(sizeof(pthread_t) * thread_num); // 乘 线程数量
    // malloc

    for (int i = 0; i < pool->thread_num; i++)
    {
        // 创建线程
        pthread_create(&pool->pthread_ids[i], NULL, (void *)threadpool_function, (void *)pool);
    }

    return pool;
}

void threadpool_add_job(struct threadpool *pool, void *(*func)(void *), void *arg)
{
    pthread_mutex_lock(&(pool->mutex));
    // 队列满
    while (pool->queue_cur_num == pool->queue_max_num)
    {
        // 阻塞等待 不满条件发生
        // 队列任务满 不得添加
        pthread_cond_wait(&pool->queue_not_full, &(pool->mutex));
    }

    // 定义函数链表
    struct job *pjob = (struct job *)malloc(sizeof(struct job));
    // malloc

    pjob->func = func;
    pjob->arg = arg;
    pjob->next = NULL;

    // pjob->func(pjob->arg);
    if (pool->head == NULL) // 队列为空
    {
        pool->head = pool->tail = pjob; // 队头队尾指向链表
        // 唤醒  告诉别人任务队列不为空
        pthread_cond_broadcast(&(pool->queue_not_emtpy));
    }
    else // 队尾向后移1个
    {
        pool->tail->next = pjob;
        pool->tail = pjob;
    }

    pool->queue_cur_num++; // 队列任务加1
    pthread_mutex_unlock(&(pool->mutex));
}

void thread_destroy(struct threadpool *pool)
{
    pthread_mutex_lock(&(pool->mutex));

    while (pool->queue_cur_num != 0)
    {
        // 等任务队列为空 才能销毁 阻塞等待 空条件
        pthread_cond_wait(&(pool->queue_empty), &(pool->mutex));
    }

    pthread_mutex_unlock(&(pool->mutex));

    // 为空 唤醒不满条件  看有没有阻塞的线程
    pthread_cond_broadcast(&(pool->queue_not_full));
    // pthread cond broadcast(&( pool->queue_not_empty));

    // 任务队列为空时  置为1 告诉其他线程要退出了
    pool->pool_close = 1;

    // 回收线程资源
    for (int i = 0; i < pool->thread_num; i++)
    {
        // 每次都唤醒  不唤醒 阻塞无法执行 线程释放
        pthread_cond_broadcast(&(pool->queue_not_emtpy));
        // pthread_cancel(pool->pthread_ids[i]); //有系统调用，才能销毁掉；有bug
        printf("thread exit!\n");
        pthread_join(pool->pthread_ids[i], NULL);
    }

    pthread_mutex_destroy(&(pool->mutex));
    pthread_cond_destroy(&(pool->queue_empty));
    pthread_cond_destroy(&(pool->queue_not_emtpy));
    pthread_cond_destroy(&(pool->queue_not_full));

    free(pool->pthread_ids);

    // 再次，释放每个节点
    struct job *temp;
    while (pool->head != NULL)
    {
        temp = pool->head;
        pool->head = temp->next;
        free(temp);
    }

    free(pool);

    printf("destroy finish!\n");
}
