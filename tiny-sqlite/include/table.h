#ifndef _TABLE_H_
#define _TABLE_H_

#include "config.h"
#include "page.h"

typedef struct {
    Pager *pager;
    uint32_t root_page_num; // root所在页的索引
} Table; 

// 创建表
Table* db_open(const char* file_name);

// 释放
void db_close(Table* table);

// 获取该行写入的地址
void* row_slot(Table *table, uint32_t row_num);
#endif