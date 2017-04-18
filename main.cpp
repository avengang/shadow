#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string>
#include "util/socketHelper.h"

struct ps
{
    int st;
    pthread_t *thr;
};
//静态多线程初始化
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//只能开启一个client
int status = 0;

void *recvsocket(void *arg)//接收client端socket数据的线程
{
    struct ps *p = (struct ps *)arg;
    int st = p->st;
    char s[1024];

    while(1)
    {
        memset(s, 0, sizeof(s));
        int rc = recv(st, s, sizeof(s), 0);
        if (rc <= 0)//如果recv返回小于等于0，代表socket已经关闭或者出错了
            break;
        printf("client：%s", s);

    }
    pthread_mutex_lock(&mutex);
    status = 0;
    pthread_mutex_unlock(&mutex);
    pthread_cancel(*(p->thr));//被cancel掉的线程内部没有使用锁。
    return NULL;
}

void *sendsocket(void *arg)//向client端socket发送数据的线程
{
    int st = *(int *)arg;
    char s[1024];
    while(1)
    {
        memset(s, 0, sizeof(s));
        read(STDIN_FILENO, s, sizeof(s));//从键盘读取用户输入信息
        send(st, s, strlen(s), 0);
    }
    return NULL;
}

int main(int arg, char *args[]) {

    int st = socket(AF_INET, SOCK_STREAM, 0);//初始化socket
    int on = 1;
         //IP可重用，关掉程序还能启动同个IP聊天
    if (setsockopt(st, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
        printf("setsockopt failed %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    struct sockaddr_in addr;     //定义一个IP地址结构
    SocketHelper::getAndBindSockaddr_in(st, addr, "127.0.0.1", 8080);

    //server端开始listen，
    if (listen(st, 20) == -1)
    {
        printf("listen failed %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    int client_st = 0;      //client端socket
    struct sockaddr_in client_addr;//表示client端的IP地址

    pthread_t thrd1, thrd2;

    while (1)
    {
        memset(&client_addr, 0, sizeof(client_addr));
        socklen_t len = sizeof(client_addr);
        //accept会阻塞，直到有客户端连接过来，accept返回client的socket描述符
        client_st = accept(st, (struct sockaddr *)&client_addr , &len);
        pthread_mutex_lock(&mutex);//为全局变量加一个互斥锁，防止与线程函数同时读写变量的冲突
        status++;
        pthread_mutex_unlock(&mutex);//解锁
        if (status > 1)//代表这是第二个socket连接
        {
            close(client_st);
            continue;
        }
        if (client_st == -1)
        {
            printf("accept failed %s\n", strerror(errno));
            return EXIT_FAILURE;
        }

        printf("accept by %s\n", inet_ntoa(client_addr.sin_addr));
        struct ps ps1;
        ps1.st = client_st;
        ps1.thr = &thrd2;
        pthread_create(&thrd1, NULL, recvsocket, &ps1);
        pthread_detach(thrd1);//设置线程为可分离
        pthread_create(&thrd2, NULL, sendsocket, &client_st);
        pthread_detach(thrd2);//设置线程为可分离
    }
    close(st);//关闭server端listen的socket
    return EXIT_SUCCESS;
}
