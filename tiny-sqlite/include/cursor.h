#ifndef _CURSOR_H_
#define _CURSOR_H_

#include "config.h"
#include "table.h"


typedef struct {
    Table* table;
    uint32_t page_num;
    uint32_t cell_num;
    bool end_of_table; 
} Cursor; 

// 获取表起始 Cursor
Cursor* table_start(Table* table);

// 获取表结束 Cursor
Cursor* table_end(Table* table);

// 如果key 存在, 返回所在游标；
// 如果不存在, 返回需要插入的位置；
Cursor* table_find(Table* table, uint32_t key_to_insert);

// 移动游标
void cursor_advance(Cursor* cursor); 

void* cursor_value(Cursor* cursor);
#endif