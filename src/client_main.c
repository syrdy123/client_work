#include <sys/socket.h>
#include <stdbool.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/stat.h>   
#include <sys/types.h> 

#include "client_log.h"


static bool is_running = true;
static int file_count = 0;

#define FILE_DIRECTORY "file/"
#define FILE_PREFIX     "log"
#define REGULAR_FILE_SUFFIX ".txt"
#define ZIP_FILE_SUFFIX     ".zip"



#define BUF_SIZE 8192


enum FILE_TYPE{
    REGULAR_FILE    = 0,   //普通文件
    COMPRESSED_FILE = 1    //压缩文件
};


void sig_handle(int sig){
    is_running = false;
    LOG(LOG_DEBUG, "client exited!");

}

void usage(const char* program_name){
    printf("Usage:\n"
            "%s ip port\n"
            "such as: ./%s 192.168.245.128 8888\n", program_name, program_name);
}


void ensure_directory_exists(const char* dir) {
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        if (mkdir(dir, 0700) != 0) {
            LOG(LOG_ERROR, "failed to create directory %s: %s", dir, strerror(errno));
        }
    }
}

int create_socket_and_connect(const char* ip_str, int port){
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        LOG(LOG_ERROR, "failed to create fd: %s", strerror(errno));
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip_str, &server_addr.sin_addr);

    if(connect(fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        LOG(LOG_ERROR, "failed to connect %s:%d, %s!", ip_str, port, strerror(errno));
        close(fd);
        return -1;
    }

    return fd;
}

void write_file(const char* filename, const char* buf, int len){
    int file_fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (file_fd < 0) {
        LOG(LOG_ERROR, "failed to open file %s: %s", filename, strerror(errno));
        return;
    }
    
    ssize_t written = write(file_fd, buf, len);
    if (written < 0) {
        LOG(LOG_ERROR, "failed to write to file %s: %s", filename, strerror(errno));
    }

    close(file_fd);
}

int main(int argc, char* argv[]){
    
    const char* program_name = basename(argv[0]);

    if(argc != 3){
        usage(program_name);
        exit(EXIT_FAILURE);    
    }

    signal(SIGINT, sig_handle);
    const char* ip_str = argv[1];
    int port = atoi(argv[2]);
    char buf[BUF_SIZE] = {0};
    char filename[128] = {0};
    bool is_first_recv = true;

    //确保目录是否存在
    ensure_directory_exists(FILE_DIRECTORY);

    while(is_running){
        int fd = create_socket_and_connect(ip_str, port);
        if(fd < 0){
            sleep(2);
            continue;
        }

        int recvBytes = 0;
        is_first_recv = true;
        while((recvBytes = recv(fd, buf, sizeof(buf), 0)) > 0){
            //第一次接收，需要根据第一个字节判断是普通文件还是压缩文件
            if(is_first_recv){
                
                if (buf[0] == REGULAR_FILE) { // 普通文件
                    snprintf(filename, sizeof(filename), "%s%s%d%s", FILE_DIRECTORY, FILE_PREFIX, file_count++, REGULAR_FILE_SUFFIX);
                } 
                else if (buf[0] == COMPRESSED_FILE) { // 压缩文件
                    snprintf(filename, sizeof(filename), "%s%s%d%s", FILE_DIRECTORY, FILE_PREFIX, file_count++, ZIP_FILE_SUFFIX);
                }

                is_first_recv = false;

                write_file(filename, buf + 1, recvBytes - 1);
            }
            else{
                write_file(filename, buf, recvBytes);
            }
        }

        close(fd);
    }

    return 0;
}