#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>


struct block_meta
{
  size_t size;
  struct block_meta *next;
  int free;
}; 

#define META_SIZE sizeof(struct block_meta) // sizeof(struct block_meta) = 24

void *global_base = NULL;


struct block_meta *find_free_block(struct block_meta **last, size_t size);  /* 按地址序, 搜索满足条件的空闲块 */
struct block_meta *request_space(struct block_meta *last, size_t size);     /* 申请内存 */
void *my_malloc(size_t size);                                               /* 简易实现 malloc */
struct block_meta *get_block_ptr(void *ptr);                                /* 由实内容的地址，获取 struct block_meta 头地址*/
void *get_real_ptr(struct block_meta *ptr);                                 /* 由struct block_meta 头地址，获得实内容的地址 */
void my_free(void *ptr);                                                    /* 模拟实现内存回收 */
void *my_realloc(void *ptr, size_t size);                                   /* 实现简易版 realloc */


/**
 * 从列表开始位置搜索，找到满足 (空闲 && 大小合适) 的空闲块
 *  last: 记录目的 block 的上一block，用于设置 next；
 *  size: 不含 struct block_meta 的 size
*/
struct block_meta *find_free_block(struct block_meta **last, size_t size)
{
  struct block_meta *current = global_base;
  while (current && !(current->free && current->size >= size))
  {
    *last = current;
    current = current->next;
  }
  return current;
}

/**
 * 内部使用 sbrk 申请内存
 *  size：不含 struct block_meta 的 size
*/
struct block_meta *request_space(struct block_meta *last, size_t size)
{
  void *old_addr;
  old_addr = sbrk(0); // 原地址 

  void *request = sbrk(size + META_SIZE);
  assert(old_addr == request);   // Not thread safe.
  if (request == (void *)-1)
  {
    return NULL; // sbrk failed.
  }

  struct block_meta* block = (struct block_meta*)old_addr;
  if (last)
  {
    last->next = block;
  }
  block->size = size;
  block->next = NULL;
  block->free = 0;
  return block;
}

/**
 * 简易实现 malloc 
*/
void *my_malloc(size_t size)
{
  struct block_meta *block;

  if (size <= 0)
  {
    return NULL;
  }

  if (!global_base)
  {
    // 首次调用
    block = request_space(NULL, size);
    if (!block)
    {
      return NULL;
    }
    global_base = block;
  }
  else
  {
    // 非首次调用
    struct block_meta *last = global_base;
    block = find_free_block(&last, size);
    if (!block)
    {
      // 没有找到可用的空闲块
      block = request_space(last, size);
      if (!block)
      {
        return NULL;
      }
    }
    else
    {
      // 找到可用的空闲块
      block->free = 0;
    }
  }
  return (block + 1);
}

/**
 * 由实内容的地址，获取 struct block_meta 头地
*/
struct block_meta *get_block_ptr(void *ptr)
{
  return (struct block_meta *)ptr - 1;
}


/**
 * 由struct block_meta 头地址，获得实内容的地址
*/
void *get_real_ptr(struct block_meta *ptr) {
  return (ptr + 1);
}


/**
 * 通过真正内容的地址，标记 struct block_meta.free 为 真
*/
void my_free(void *ptr)
{
  if (!ptr)
  {
    return;
  }
  
  struct block_meta *block_ptr = get_block_ptr(ptr);
  assert(block_ptr->free == 0);
  block_ptr->free = 1;
  memset(ptr, 0, block_ptr->size);
}

/**
 *  实现简易版 realloc
*/
void *my_realloc(void *ptr, size_t size)
{
  if (!ptr)
  {
    return my_malloc(size);
  }

  struct block_meta *block_ptr = get_block_ptr(ptr);
  if (block_ptr->size >= size)
  {
    return ptr;
  }

  void *new_ptr;
  new_ptr = my_malloc(size);
  if (!new_ptr)
  {
    return NULL; // TODO: set errno on failure.
  }
  memcpy(new_ptr, ptr, block_ptr->size);
  my_free(ptr);
  return new_ptr;
}

void log_mem() {
  struct block_meta *cur = global_base;
  while (cur) {
    printf("addr: %p, is_free: %d, size: %ld, content: %s\n", cur, cur->free, cur->size, (const char*)get_real_ptr(cur));
    cur = cur->next;
  }
}

int main()
{
  void* p1 = my_malloc(16);
  strcpy(p1, "hello");

  void *p2 = my_malloc(8);
  strcpy(p2, "my");

  void *p3 = my_malloc(64);
  strcpy(p3, "friend");

  log_mem();

  my_free(p2);
  void *p4 = my_malloc(4);
  strcpy(p4, "MY");
  log_mem();

  void *p5 = my_realloc(p4, 12);
  strcpy(p5, "you are my");
  log_mem();

  return 0;
}