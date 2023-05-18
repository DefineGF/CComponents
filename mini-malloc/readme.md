### mini - malloc

##### 前言

```c
#define SIZE 16
// 申请内存
void* p = malloc(SIZE);
// 释放内存
free(p);
```

- 申请内存时，指定申请内存的大小，并返回申请内存的起始地址；

- 释放内存时，只需要传入内存的起始地址即可，无需传入需要释放内存的大小。

具体原理无非是申请 SIZE 内存时候，分配的内存实质上是一小段 cookie + SIZE，只不过返回SIZE对应的起始地址；释放时候再根据 cookie 大小偏移，获取cookie中记录的内存的大小进行释放。相关文章有很多，具体直接google即可！



#### 简单实现

##### 头文件

```c
#include <stdio.h>	// printf ...
#include <string.h> // strcpy ...
#include <assert.h> // assert ...
#include <unistd.h> // sbrk   ...
```

##### 结构体及相关函数

模拟malloc中的cookie：

```c
struct block_meta
{
  size_t size;				// 记录内容部分占用大小
  struct block_meta *next;  // 下一个申请的内存块
  int free;					// 记录当前block 是否空闲
}; 
#define META_SIZE sizeof(struct block_meta) // sizeof(struct block_meta) = 24
```



全局变量：void *global_base = NULL; 用于记录申请的block的头

```c
struct block_meta *find_free_block(struct block_meta **last, size_t size);  /* 按地址序, 搜索满足条件的空闲块 */
struct block_meta *request_space(struct block_meta *last, size_t size);     /* 申请内存 */
void *my_malloc(size_t size);                                               /* 简易实现 malloc */
struct block_meta *get_block_ptr(void *ptr);                                /* 由实内容的地址，获取 struct block_meta 头地址*/
void *get_real_ptr(struct block_meta *ptr);                                 /* 由struct block_meta 头地址，获得实内容的地址 */
void my_free(void *ptr);                                                    /* 模拟实现内存回收 */
void *my_realloc(void *ptr, size_t size);                                   /* 实现简易版 realloc */
```



##### void *my_malloc(size_t size)

整体流程如下：

- 如果是首次调用（即还未有内存块），则直接调用 request_space 申请内存，并设置链表；

- 如果不是首次调用，首先需要通过 find_free_block 查找block 链中是否有 （空闲&& 大小合适）的 block：

  - 有，则直接返回空闲地址；
  - 没有，则调用 request_space  申请；

  

最终返回的是不含 struct block_meta 头的首地址！具体实现如下：

```c
void *my_malloc(size_t size)
{
  if (size <= 0)
  {
    return NULL;
  }
    
  struct block_meta *block;
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
  return (block + 1); // 这里 + 1表示返回偏移 struct block_meta，即真正内容的首地址（不含 cookie 头）
}
```



##### struct block_meta *request_space(struct block_meta *last, size_t size)

核心逻辑通过 sbrk 来分配内存，具体可参见sbrk 和 brk 相关原理！ 具体实现如下：

```c
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
```

根据第 6 行可见，最终申请的是 size + META_SIZE （sizeof(struct block_meta）) 大小的内存！

返回给 my_malloc 的也是指向struct block_meta 头的地址，因此需要在 my_mallioc 中对地址进行偏移处理！



##### struct block_meta *find_free_block(struct block_meta **last, size_t size)

自首节点进行遍历查找满足条件的空闲 block，并设置上一 block，即last，用以进一步设置 next 指针！详情如下：

```c
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
```



##### void my_free(void *ptr)

这里并没有调用brk进行相应的内存释放，而是简单的将block中的free设置为1，标记为空闲即可！便于后面申请内存时重复使用！具体实现如下：

```c
void my_free(void *ptr)
{
  if (!ptr)
  {
    return;
  }
  
  struct block_meta *block_ptr = get_block_ptr(ptr);
  assert(block_ptr->free == 0);
  block_ptr->free = 1;
  memset(ptr, 0, block_ptr->size);  // 清空内容
}
```



##### void *my_realloc(void *ptr, size_t size)

流程如下：

- 当ptr指向为NULL，调用my_malloc申请 size 大小的内存！
- 当ptr指向内容非NULL时：
  - 如果ptr指向的block大于（满足）指定的size，则直接重复使用；
  - 如果不是，则重新申请内容，并将内容复制到新内存中，同时调用my_free 释放原内容！



##### 其他函数

```c
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

// 打印所有申请的内存块信息
void log_mem() {
  struct block_meta *cur = global_base;
  while (cur) {
    printf("addr: %p, is_free: %d, size: %ld, content: %s\n", cur, cur->free, cur->size, (const char*)get_real_ptr(cur));
    cur = cur->next;
  }
}
```



#### 测试

```c
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
```



#### 索引

引用：https://danluu.com/malloc-tutorial/

