# 编译器
CC = gcc

# 编译选项
CFLAGS = -Wall -g -Iinclude

# 可执行文件名
TARGET = client

# 源文件和目标文件路径
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:src/%.c=src/%.o)

# 默认目标
all: $(TARGET)

# 生成可执行文件
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# 生成目标文件
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# 清理生成的文件
clean:
	rm -f src/*.o $(TARGET)

.PHONY: all clean
