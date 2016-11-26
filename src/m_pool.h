/**
 *
 * 基于各种尺寸大小的空闲块链实现的内存池
 *
 * @file m_pool.h
 * @author chosen0ne
 * @date 2012-04-07
 */

#ifndef SHMMAP_M_POOL_H
#define SHMMAP_M_POOL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#define padding(p) *((int *)p) = 0
#define NIL -1
/* 允许内存块的最大值为 8*128*1024 = 1MB */
#define FREE_LIST_SIZE 128*1024

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	SHMMAP_LOG_DEBUG,
	SHMMAP_LOG_INFO,
	SHMMAP_LOG_WARN,
	SHMMAP_LOG_ERROR,
    SHMMAP_LOG_LEVEL_NUM
} shmmap_log_level;

/* 记录日志 */
typedef void (*shmmap_log)(shmmap_log_level level, const char *fmt, ...);

/*
	空闲块大小是8bytes的倍数，便于字节对齐
	8:
	16:
	...
	8n:
	对于每个空闲块大小都有一个链表，组织所有的空闲块。
	对于一个内存申请请求，需要提供所请求内存的大小len，
	在空闲块列表free_list中查找恰好可以容纳len的空闲块链n8_list。
	然后在n8_list中返回空闲块。
	空闲块列表，就是数组：
	-----------
	| 8bytes  | --> block --> block --> block
	|----------
	| 16bytes | --> block
	|----------
	|	...	  |
	|---------|
	| 8nbytes |
	-----------
	空闲块的结构：
	{--------------头部-----------------}
	-------------------------------------------------
	| index | prev | next | data length | 	data	|
	-------------------------------------------------

 */

/*
 * 空闲块的头部
 */
typedef struct m_block_hdr {
	int idx;
	int prev_offset;
	int next_offset;
	int data_len;
} M_block_hdr;

// 分配内存时从header删除m_node，回收内存时在tail添加m_node
typedef struct m_header {
	int header_offset;
	int tail_offset;
	int size;		// 链表的长度
	int idx;
} M_header;

typedef struct m_mem_info {
	int pool_size;
	int free_area_size;
	int allocated_area_size;
	int real_used_size;
	int allocated_area_free_size;
} M_mem_info;
/**
 * 内存池初始化
 * pool_ptr: 		内存块起始地址
 * pool_size: 		内存块的大小
 * log				日志handler
 * is_inited: 		是否已经初始化。已经初始化则直接读取索引，否则建立索引。
 */
bool m_init(char *pool_ptr, int pool_size, shmmap_log log, bool is_inited);
/* 申请可以容纳len bytes的内存块 */
void* m_alloc(int len);
/* 释放p指向的内存块 */
void m_free(void *p);


//**********************内存使用状况**********************//
/* 返回空闲内存大小，bytes */
int m_free_size();
/* 整个内存池大小 */
int m_pool_size();
/* 返回空闲列表信息 */
void m_free_list_info();
void m_memory_info(M_mem_info *info);


//**********************向内存块填充内容******************//
/* 根据空闲块中data字段起始地址设置对应的data数据 */
void set_mnode_data_by_data(void *data_ptr, void *data_content_ptr, int len);


//**********************指针、偏移量**********************//
void* get_ptr(int offset);
int ptr_offset(void *p);


//**********************默认日志输出handler***************//
void default_shmmap_log(shmmap_log_level level, const char *msg, ...);

#ifdef __cplusplus
}
#endif

#endif
