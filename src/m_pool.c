/**
 *
 * 基于各种尺寸大小的空闲块链实现的内存池
 *
 * @file m_pool.c
 * @author chosen0ne
 * @date 2012-04-07
 */

#include "m_pool.h"
#include<assert.h>

static int M_HEADER_SIZE = sizeof(M_header);
static int BLOCK_HEADER_SIZE = sizeof(M_block_hdr);	// 空闲块头部大小
static int INT_SIZE = sizeof(int);
static int PTR_SIZE = sizeof(void *);

static M_header* free_list = NULL;	// 空闲块链
static int free_list_len;			// 空闲块链长度
static void *pool_ptr_s;			// 共享内存的起始地址
static void *pool_ptr_e;			// 共享内存的结束地址
static int pool_byte_size;			// 内存池包含的字节数
static int *current_p_offset;		// 当前空闲区的起始地址距离内存池起始地址的偏移量
static shmmap_log m_pool_log;		// 日志handler

static char *log_level_labels[SHMMAP_LOG_LEVEL_NUM] = {"DEBUG", "INFO", "WARN", "ERROR"};

/* 根据申请的内存大小返回对应的空闲块链 */
static int free_list_idx(int size);
/* 根据空闲块的起始地址获取该空闲块对应的数据字段的起始地址*/
static int get_mnode_data(int p);
/* 根据空闲块中data字段的起始地址获取对应的空闲块的地址 */
static int get_mnode_by_data(int p);
static int get_mnode_by_data_ptr(void *p);
static int m_free_blck_size();
static void* get_cur_ptr();
static void set_cur_ptr_offset(int offset);

/* 根据块大小找到对应的链表在free_list中的下标 */
static int
free_list_idx(int size){
	if(size % 8 == 0)
		return (size >> 3) - 1;
	return size >> 3;
}

/* 根据空闲块的偏移量返回数据字段偏移量 */
static int
get_mnode_data(int block_ptr){
	int data_offset = block_ptr + BLOCK_HEADER_SIZE;
	if(data_offset > pool_byte_size){
		m_pool_log(SHMMAP_LOG_ERROR, "[get_mnode_data]The offset(%d) of data ptr is bigger than pool_byte_size(%d)", data_offset, pool_byte_size);
		return -1;
	}
	return data_offset;
}

/* 根据数据字段的偏移量获取整个块的偏移量 */
static int
get_mnode_by_data(int data_offset){
	int block_offset = data_offset - BLOCK_HEADER_SIZE;
	if(block_offset < 0){
		m_pool_log(SHMMAP_LOG_ERROR, "[get_mnode_by_data]The offset(%d) of block ptr is lesser than 0");
		return -1;
	}
	return block_offset;
}

/* 根据数据字段的指针获取块的偏移量 */
static int
get_mnode_by_data_ptr(void *p){
	int offset = ptr_offset(p);
	return get_mnode_by_data(offset);
}

static void*
get_cur_ptr(){
	return (char *)pool_ptr_s + *current_p_offset;
}

static void
set_cur_ptr_offset(int offset){
	*current_p_offset = *current_p_offset + offset;
}

/**
 * 分配大小为size字节的内存
 * return: 返回距离内存池起始地址的偏移量
 */
static int
_m_alloc(int size){
	M_header 		*hdr;
	M_block_hdr 	*p, *q;
	int 			idx, p_offset, q_offset;

	idx = free_list_idx(size);
	if(idx >= free_list_len){
		m_pool_log(SHMMAP_LOG_ERROR, "[_m_alloc]The max block size is %d, the request is too large %d", free_list_len<<3, size);
		return -1;
	}
	hdr = &free_list[idx];

	if(hdr->size > 0){
		// 有空闲块链，直接分配内存，从header删除m_node
		p_offset = hdr->header_offset;
		p = (M_block_hdr *)get_ptr(p_offset);
		q_offset = p->next_offset;
		if(q_offset != NIL){
			q = (M_block_hdr *)get_ptr(q_offset);
			q->prev_offset = NIL;
		}
		hdr->header_offset = q_offset;
		hdr->size--;
		return get_mnode_data(p_offset);
	}else{
		// 没有空闲块时，直接从空闲内存分配
		int chunck_size = (idx+1) << 3;
		int block_size = BLOCK_HEADER_SIZE + chunck_size;
		if(*current_p_offset + block_size > pool_byte_size){
			m_pool_log(SHMMAP_LOG_ERROR, "[_m_alloc]The unallocated area is used up, the size of free space is %d",
				m_free_size());
			return -1;
		}
		p = (M_block_hdr *)get_cur_ptr();
		// 标记这个内存块对应的空闲块列表的下标
		p->idx = idx;

		set_cur_ptr_offset(block_size);
		padding(get_cur_ptr());
		set_cur_ptr_offset(INT_SIZE);
		return get_mnode_data(ptr_offset(p));
	}
}

/*
 * 释放内存
 * data_offset: 数据字段距离内存池起始地址的偏移量
 */
void
_m_free(int data_offset){
	M_block_hdr		*block_ptr, *tail_block_ptr;
	int 			idx, block_offset, tail_block_offset;
	M_header		*hdr;

	block_offset = get_mnode_by_data(data_offset);
	if(block_offset == -1){
		m_pool_log(SHMMAP_LOG_ERROR, "[_m_free]Get block offset error");
		return;
	}
	block_ptr = (M_block_hdr *)get_ptr(block_offset);
	idx = block_ptr->idx;
	if(idx<0 || idx>=free_list_len){
		m_pool_log(SHMMAP_LOG_ERROR, "[_m_free]The free block linked list dosen't exist. The length of free list is %d, and the index of the block to free is %d",
			free_list_len, idx);
		return;
	}
	hdr = &free_list[idx];
	if(hdr->size == 0){
		block_ptr->prev_offset = NIL;
		hdr->header_offset = hdr->tail_offset = block_offset;
	}else{
		// 加入空闲块链表，在tail添加
		tail_block_offset = hdr->tail_offset;
		tail_block_ptr = (M_block_hdr *)get_ptr(tail_block_offset);
		block_ptr->prev_offset = tail_block_offset;
		tail_block_ptr->next_offset = block_offset;
		hdr->tail_offset = block_offset;
	}
	block_ptr->next_offset = NIL;
	hdr->size ++;
}

bool
m_init(void *p, int pool_byte_len, shmmap_log log, bool is_inited){
	int 	i, f_list_len;
	int 	*size_p;

	// 初始化日志handler
	m_pool_log = log;
	if(log == NULL){
		m_pool_log = default_shmmap_log;
	}

	f_list_len = FREE_LIST_SIZE;
	if(pool_byte_len < INT_SIZE*2 + M_HEADER_SIZE*f_list_len){
		m_pool_log(SHMMAP_LOG_ERROR, "[m_init]The pool size is too small %d bytes，it can't allocatie memory for index %d bytes",
			pool_byte_len, INT_SIZE*2+M_HEADER_SIZE*f_list_len);
		return false;
	}

	pool_ptr_s = p;
	pool_byte_size = pool_byte_len;
	pool_ptr_e = pool_ptr_s + pool_byte_size;
	free_list_len = f_list_len;

	/**
	 * 内存池头部：
	 * Free List Len 	(4bytes)
	 * padding			(4bytes)
	 * current ptr		(PTR_SIZE)
	 * padding			(4bytes)
	 * Free List		(M_HEADER_SIZE * free_list_len)
	 */
	if(is_inited){
		p += 2 * INT_SIZE;
		current_p_offset = (int *)p;
		p += PTR_SIZE + INT_SIZE;
		free_list = (M_header *)p;
	}else{
		size_p = (int *)p;
		*size_p++ = f_list_len;
		*size_p = 0;	// padding
		p += INT_SIZE * 2;
		current_p_offset = (int *)p;
		p += PTR_SIZE;
		padding(p);
		p += INT_SIZE;
		free_list = (M_header *)p;
		for(i=0; i<free_list_len; i++){
			(free_list+i)->idx = i;
			(free_list+i)->size = 0;
		}
		p += M_HEADER_SIZE * free_list_len;
		padding(p);
		*current_p_offset = (char *)p + INT_SIZE - (char *)pool_ptr_s;
	}

	m_pool_log(SHMMAP_LOG_INFO, "[m_init]Init memory pool, address start at %p, end at %p, size is %d",
		pool_ptr_s, pool_ptr_e, pool_byte_len);

	return true;
}

/**
 * 返回空闲块的起始地址
 */
void*
m_alloc(int size){
	int ptr_offset = _m_alloc(size);
	if(ptr_offset != -1){
		return get_ptr(ptr_offset);
	}
	return NULL;
}

/**
 * 传入要释放的空闲块的data字段起始地址
 */
void
m_free(void *data_ptr){
	_m_free(ptr_offset(data_ptr));
}

/**
 * 根据数据字段的指针，设置一个块的数据字段的内容
 * data_ptr: 空闲块中数据指针
 * data_content_ptr: 要设置的数据内容的指针
 * len: 数据内容的长度，字节数
 */
void
set_mnode_data_by_data(void *data_ptr, void *data_content_ptr, int len){
	M_block_hdr *block_ptr = (M_block_hdr *)get_ptr(get_mnode_by_data_ptr(data_ptr));
	block_ptr->data_len = len;
	memcpy(data_ptr, data_content_ptr, len);
}


int
m_free_size(){
	return pool_byte_size - *current_p_offset;
}

int
m_pool_size(){
	return pool_byte_size;
}

void m_free_list_info(){
	int i;
	for(i=0; i<free_list_len; i++){
		if(free_list[i].size != 0){
			m_pool_log(SHMMAP_LOG_INFO, "[%d, %d]", (i+1)<<3, free_list[i].size);
		}
	}
}

/* 所有空闲块内存的大小 */
static int
m_free_blck_size(){
	int i, total = 0;
	for(i=0; i<free_list_len; i++){
		total += ((i+1)<<3)*(free_list + i)->size;
	}
	return total;
}


void
m_memory_info(M_mem_info *info){
	info->pool_size = pool_byte_size;
	info->free_area_size = m_free_size();
	info->allocated_area_size = pool_byte_size - info->free_area_size;
	info->allocated_area_free_size = m_free_blck_size();
	info->real_used_size = info->allocated_area_size - info->allocated_area_free_size;
}

/* 打印空闲块列表信息 */
void
m_free_info(){
	int i;
	printf("Free List INFO:\n");
	for(i=0; i<free_list_len; i++){
		if(free_list[i].size != 0)
			printf("\tindex: %d, \tsize: %d\n", free_list[i].idx, free_list[i].size);
	}
}

/* 获取指针相对于内存池起始地址的偏移量 */
int
ptr_offset(void *p){
	assert(p > pool_ptr_s);
	if(p < pool_ptr_s){
		m_pool_log(SHMMAP_LOG_ERROR, "[ptr_offset]Pointer(%p) is less than pool_ptr_s(%p).", p, pool_ptr_s);
		return -1;
	}
	return (char*)p - (char*)pool_ptr_s;
}

/* 根据偏移量获取指针 */
void*
get_ptr(int offset){
	void *p = (char*)pool_ptr_s + offset;
	assert(p > pool_ptr_s);
	assert(p < pool_ptr_e);
	if(p<pool_ptr_s || p>pool_ptr_e){
		m_pool_log(SHMMAP_LOG_ERROR, "[get_ptr]Offset(%d) must be larger than 0 and less than pool_byte_size(%d)",
			offset, pool_byte_size);
		return NULL;
	}
	return p;
}

void
default_shmmap_log(shmmap_log_level level, const char *msg_fmt, ...){
	va_list ap;
	va_start(ap, msg_fmt);
	char *fmt = malloc(strlen(msg_fmt)+20);
	sprintf(fmt, "-%s- %s\n", log_level_labels[level], msg_fmt);
	vprintf(fmt, ap);
	free(fmt);
}
