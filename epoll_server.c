#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>          // fcntl
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>      // epoll 的头文件

#define PORT 8888
#define MAX_EVENTS 1000     // 一次最多处理多少个事件

// 设置 Socket 为非阻塞模式
void setnonblocking(int sockfd) {
    int opts;
    opts = fcntl(sockfd, F_GETFL); // 获取当前标志
    if (opts < 0) {
        perror("fcntl(F_GETFL)");
        exit(1);
    }
    opts = (opts | O_NONBLOCK);    // 加上非阻塞标志
    if (fcntl(sockfd, F_SETFL, opts) < 0) {
        perror("fcntl(F_SETFL)");
        exit(1);
    }
}

int main() {
    int server_fd, client_fd, epoll_fd, event_count;
    struct sockaddr_in address;
    struct epoll_event event, events[MAX_EVENTS]; // 用来存发生的事情

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // 2. 绑定 (Bind) IP 和端口
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // 监听所有网卡接口
    address.sin_port = htons(PORT);       // 端口号转为网络字节序

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // 3. 监听 (Listen)
    // 3 表示等待队列的最大长度
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    setnonblocking(server_fd);

    // 2. 创建 Epoll 实例 (红灯箱)
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1 failed");
        exit(1);
    }

    // 3. 把 Server Socket 添加到 Epoll 监控列表
    event.data.fd = server_fd;       // 监控谁？Server
    event.events = EPOLLIN;          // 监控什么事？EPOLLIN (有数据进来了/有连接进来了)

    // epoll_ctl(红灯箱, 动作:添加, 谁, 监听什么)
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1) {
        perror("epoll_ctl: server_fd");
        exit(1);
    }

    printf("Epoll Server running on port %d...\n", PORT);

    // 4. 事件循环 (Event Loop)
    while (1) {
        // 等待事件发生 (阻塞在这里，直到至少有一个灯亮，或者超时)
        // 参数: epoll_fd, 接收结果的数组, 数组容量, 超时时间(-1表示无限等)
        event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

        if (event_count == -1) {
            perror("epoll_wait");
            exit(1);
        }

        // 遍历所有发生的事件
        for (int i = 0; i < event_count; i++) {
            int curr_fd = events[i].data.fd;

            // 情况 A: Server Socket 亮灯了 -> 说明有新连接！
            if (curr_fd == server_fd) {
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);

                client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
                if (client_fd == -1) {
                    perror("accept");
                    continue;
                }

                printf("New Client Connected: FD %d\n", client_fd);

                // 把新来的 Client 也设为非阻塞
                setnonblocking(client_fd);

                // 把这个新 Client 也加入 Epoll 监控
                event.data.fd = client_fd;
                event.events = EPOLLIN; // | EPOLLET (边缘触发，进阶选项，先不加)
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
            }
            // 情况 B: 普通 Client Socket 亮灯了 -> 说明客户端发数据来了
            else {
                char buffer[1024] = {0};
                int bytes_read = read(curr_fd, buffer, 1024);

                if (bytes_read <= 0) {
                    // 读到 0 表示客户端断开了连接，或者出错
                    // 从 Epoll 中移除
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, curr_fd, NULL);
                    close(curr_fd);
                    printf("Client Disconnected: FD %d\n", curr_fd);
                } else {
                    // 正常读取到数据
                    printf("Recv from FD %d: %s\n", curr_fd, buffer);

                    // 简单的回显 (Echo)，或者在这里调用 handle_client 解析 HTTP
                    char *resp = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nHello Epoll!";
                    send(curr_fd, resp, strlen(resp), 0);

                    // HTTP 是短连接，发完通常就关了 (为了简化，发完直接关)
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, curr_fd, NULL);
                    close(curr_fd);
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
