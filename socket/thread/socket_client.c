#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <pthread.h>

#define SERVER_PORT 8888
#define SERVER_IP   "192.168.0.11"

struct sockaddr_in server_addr = {0};

int sockfd;
int connfd;
char send_buf[512] = {0};
char recv_buf[512] = {0};

bool running = true;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void *send_thread(void *argc)
{
    int ret;
    //printf("send_thread created\n");
    while (running) {
        memset(send_buf, 0, sizeof(send_buf));
        fgets(send_buf, sizeof(send_buf), stdin);
        ret = send(sockfd, send_buf, sizeof(send_buf), 0);
        if (ret < 0) {
            perror("send err");
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

static void *recv_thread(void *argc)
{
    int ret;
    //printf("recv_thread created\n");
    while (running) {
        memset(recv_buf, 0, sizeof(recv_buf));
        ret = recv(sockfd, recv_buf, sizeof(recv_buf), 0);
        if (ret < 0) {
            perror("recvv err");
            close(connfd);
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        printf("接受数据:%s", recv_buf);
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

int main(int argc, char **argv)
{
    int ret;
    pthread_t send_tid;
    pthread_t recv_tid;
    if (argc != 2) {
        printf("usage:%s <server_ip>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket err");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);          // 端口
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr); // IP

    ret = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret < 0) {
        perror("connect err");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("连接到服务器！\n");

    ret = pthread_create(&send_tid, NULL, send_thread, NULL);
    if (ret < 0) {
        perror("pthread_create err");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    ret = pthread_create(&recv_tid, NULL, recv_thread, NULL);
    if (ret < 0) {
        perror("pthread_create err");
        pthread_cancel(send_tid);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    pthread_join(send_tid, NULL);
    pthread_join(recv_tid, NULL);
    pthread_mutex_destroy(&mutex);
    close(sockfd);
    exit(EXIT_SUCCESS);
}