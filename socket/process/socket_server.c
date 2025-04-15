#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <signal.h>

#define SERVER_PORT 8888

struct sockaddr_in server_addr = {0};
struct sockaddr_in client_addr = {0};
socklen_t client_addr_len = sizeof(client_addr);

int sockfd;
int connfd;
char send_buf[512] = {0};
char recv_buf[512] = {0};

volatile bool running = 1;

void handle_signal(int sig)
{
    if (sig == SIGTERM) {
        printf("收到退出信号，正在关闭%d...\n", getpid());
        running = 0; // 设置标志位，通知主循环退出
        if(sockfd>=0){
            close(sockfd);
        }
    }
}

int main()
{
    int ret;

    signal(SIGTERM, handle_signal); // 设置信号处理函数
    signal(SIGCHLD, SIG_IGN);

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

    while (running) {
        connfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len); // client_addr为传出数据
        if (connfd < 0&&running == 1) {
            perror("accept err");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        if(running)
            printf("客户端接入！\n");

        if (fork() > 0) { // 父进程
            close(connfd);
            continue;
        }
        else { // 子进程
            close(sockfd);
            while (running) {
                memset(recv_buf, 0, sizeof(recv_buf));
                ret = recv(connfd, recv_buf, sizeof(recv_buf), 0);
                if (ret < 0) {
                    perror("recv err");
                    close(connfd);
                    exit(EXIT_FAILURE);
                }
                printf("接收数据:%s", recv_buf);
                if (strncmp(recv_buf, "exit all", 8) == 0) {
                    printf("关闭服务器中！\n");
                    kill(0, SIGTERM); // 向当前进程组所有进程发信号，父子进程均退出，不会导致孤儿进程
                    kill(getppid(), SIGTERM);
                    break;
                }
                else if (strncmp(recv_buf, "exit", 4) == 0) {
                    printf("关闭连接\n");
                    break;
                }
            }
            close(connfd);
            exit(EXIT_SUCCESS);
        }
    }
    printf("服务器已关闭！\n");
    exit(EXIT_SUCCESS);
}