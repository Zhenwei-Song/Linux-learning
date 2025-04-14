#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <pthread.h>

#define SERVER_PORT 8888

struct sockaddr_in server_addr = {0};
struct sockaddr_in client_addr = {0};
socklen_t client_addr_len = sizeof(client_addr);

int sockfd;
int connfd;
char send_buf[512] = {0};
char recv_buf[512] = {0};

bool running = true;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void *send_thread(void *arg)
{
    int ret;
    while (running) {
        memset(send_buf, 0, sizeof(send_buf));
        //printf("输入数据：\n");
        fgets(send_buf, sizeof(send_buf), stdin);
        ret = send(connfd, send_buf, sizeof(send_buf), 0);
        if (ret < 0) {
            perror("send err");
            close(connfd);
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        if (strncmp(send_buf, "exit", 4) == 0) {
            printf("关闭连接\n");
            pthread_mutex_lock(&mutex);
            running = false;
            pthread_mutex_unlock(&mutex);
            break;
        }
    }
    return NULL;
}

static void *recv_thread(void *arg)
{
    int ret;
    while (running) {
        memset(recv_buf, 0, sizeof(recv_buf));
        ret = recv(connfd, recv_buf, sizeof(recv_buf), 0);
        if (ret < 0) {
            perror("recvv err");
            close(connfd);
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        printf("接收数据:%s", recv_buf);
        if (strncmp(recv_buf, "exit", 4) == 0) {
            printf("关闭连接\n");
            pthread_mutex_lock(&mutex);
            running = false;
            pthread_mutex_unlock(&mutex);
            break;
        }
    }
    return NULL;
}

int main()
{
    int ret;
    pthread_t send_tid;
    pthread_t recv_tid;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket err");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    ret = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret < 0) {
        perror("bind err");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    ret = listen(sockfd, 50);
    if (ret < 0) {
        perror("listen err");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("等待连接...\n");
    connfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len); // client_addr为传出数据
    if (connfd < 0) {
        perror("accept err");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("客户端接入！\n");

    ret = pthread_create(&send_tid, NULL, send_thread, NULL);
    if (ret < 0) {
        perror("pthread_create err");
        close(connfd);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    ret = pthread_create(&recv_tid, NULL, recv_thread, NULL);
    if (ret < 0) {
        perror("pthread_create err");
        pthread_cancel(send_tid);
        close(connfd);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    pthread_join(send_tid, NULL);
    pthread_join(recv_tid, NULL);
    pthread_mutex_destroy(&mutex);

    close(connfd);
    close(sockfd);
    exit(EXIT_SUCCESS);
}