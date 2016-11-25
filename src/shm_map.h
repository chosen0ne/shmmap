#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/mman.h>
#include<sys/stat.h>
#include<assert.h>
#include<errno.h>
#include "m_pool.h"

#define FILE_MODE   (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define DATA_FILE "shm_map.dat"

/*
 * hash链表的节点
 */
typedef struct entry {
	int prev_offset;
	int next_offset;
	int hash;
	int key_offset;
	int value_offset;
} H_entry;

/*
 * hash的桶
 */
typedef struct bulk {
	int header_offset;
	int tail_offset;
	int size;
} H_bulk;

typedef void (*key_iter)(const char *k, const char *v);

/*
 * 初始化map
 * capacity: map的容量
 * mem_size: map占用的内存大小，就是共享内存的大小
 * dat_file_path: 数据文件存储的位置
 * log: 日志handler
 */
bool map_init(int capacity, int mem_size, const char *dat_file_path, shmmap_log log);
char* map_put(const char *k, const char *v);
char* map_get(const char *k);

int map_size();
bool map_contains(const char *k);
void map_iter(key_iter);
