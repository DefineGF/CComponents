#ifndef _PAGE_H_
#define _PAGE_H_

#include "config.h"

typedef struct {
    int file_descirptor;
    uint32_t file_len;
    uint32_t num_pages; // 记录当前使用 page 的数量
    void *pages[TABLE_MAX_PAGES];
} Pager;

Pager* pager_open(const char *file_name);

void* get_page(Pager* pager, uint32_t page_num);

uint32_t get_unused_page_num(Pager* pager);

void pager_flush(Pager *pager, uint32_t page_num);

#endif