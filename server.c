#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h> //POSIX multi-thread 

#define PORT 8888
#define BUFFER_SIZE 1024

void handle_client(int client_socket);

void *thread_function(void *arg) {
	//1.acquiring input socket
	//arg type convert
	int client_socket = *((int *)arg);
	//free malloc mem
	free(arg);

	//2.detached threads process
	pthread_detach(pthread_self());

	//3.call logics
	handle_client(client_socket);

	return NULL;
}

void handle_client(int client_socket) {
	char buffer[BUFFER_SIZE] = {0};
	//1.read requests from client
	read(client_socket, buffer, BUFFER_SIZE);

	char *method = strtok(buffer, " ");
	char *path = strtok(NULL, " ");

	if (path ==NULL) {
		close(client_socket);
		return;
	}

	//2.deal with the path
	char file_path[256];
	if (strcmp(path, "/") == 0){
		strcpy(file_path, "myshell.c");
	}else {
		//remove "/" to make a relative path
		strcpy(file_path, path + 1);
	}
	printf("client requested: %s\n",file_path);

	//3.opening files
	FILE *file = fopen(file_path, "r");
        if (file == NULL) {
		char *not_found = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\n\nFile Not Found!";
		send(client_socket, not_found, strlen(not_found), 0);
	}else {
		char *header = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\n";
		send(client_socket, header, strlen(header), 0);

		//cycle-reading files
		char file_buf[1024];
		size_t n;
		while ((n =fread(file_buf, 1, sizeof(file_buf), file)) > 0) {
			send(client_socket, file_buf, n, 0);
		}
		fclose(file);
	}
	//4.close connection
	close(client_socket);
}


int main() {

     int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    
    // 1. 创建 Socket 文件描述符
    // AF_INET = IPv4, SOCK_STREAM = TCP
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    //rnable port being recycled by former socket
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // 2. bind port and IP of server_fd before
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // 监听所有网卡接口
    address.sin_port = htons(PORT);       // 端口号转为网络字节序

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // 3. Listen--where 3 handshakes happen
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Server is listening on port %d...\n", PORT);

    while(1) {
	//accept new connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        printf("New connection established! Socket FD: %d\n", new_socket);

	//pthread program
	//1.allocate a new space of mem for socket
	int *pclient = malloc(sizeof(int));
	*pclient = new_socket;

	//2.create thread
	pthread_t t_id;
	if(pthread_create(&t_id, NULL, thread_function, pclient) != 0) {
		perror("Thread create failed");
		//rem to free mem even if failed
		free(pclient);
		close(new_socket);
	}

    }
}
