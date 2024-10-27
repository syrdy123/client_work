#ifndef CLIENT_LOG_H_
#define CLIENT_LOG_H_

#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#define LOG_INFO    1
#define LOG_DEBUG   2
#define LOG_ERROR   3

// 设置日志级别
#define LOG_LEVEL LOG_INFO

// 获取当前时间
static void get_current_log_time(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

// 日志宏定义
#define LOG(level, fmt, ...) do { \
    if (level >= LOG_LEVEL) { \
        char time_buffer[20]; \
        get_current_log_time(time_buffer, sizeof(time_buffer)); \
        const char *level_str = (level == LOG_INFO) ? "INFO" : \
                                 (level == LOG_DEBUG) ? "DEBUG" : "ERROR"; \
        printf("[%s] [%s] [%s:%d] " fmt "\n", time_buffer, level_str, __FILE__, __LINE__, ##__VA_ARGS__); \
    } \
} while (0)

#endif