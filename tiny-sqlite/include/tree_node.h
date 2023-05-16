#ifndef _TREE_NODE_H_
#define _TREE_NODE_H_

#include "config.h"
#include "cursor.h"
#include "record.h"

typedef enum {
    NODE_INTERNAL,
    NODE_LEAF,
} NodeType;


NodeType get_node_type(void* node);

// 判断节点是否是根节点
bool is_node_root(void* node);

// 设置节点是否是根节点
void set_node_is_root(void* node, bool is_root);

// 获取父节点指针
uint32_t* node_parent(void* node);

// 创建新的根节点
void create_new_root(Table* table, uint32_t right_page_num);

// 获取叶子节点中的 Cell 个数
uint32_t* leaf_node_num_cells(void* node);

// 根据 cell_num 索引计算对应的 cell 起始地址
void* leaf_node_cell(void* node, uint32_t cell_num);    

// 根据 cell_num 计算对应起始地址就是 cell 开始的key地址
uint32_t* leaf_node_key(void* node, uint32_t cell_num); 

uint32_t* leaf_node_next_leaf(void* node);

// 对应 cell 起始地址 + key偏移 = Row 起始地址
void* leaf_node_value(void* node, uint32_t cell_num);   

void initial_leaf_node(void* node);

void initial_internal_node(void* node);

Cursor* leaf_node_find(Table* table, uint32_t page_num, uint32_t key); // 二分法搜索插入位置

// 叶子节点插入
void leaf_node_insert(Cursor* cursor, uint32_t key, Row* value);

/*
  分裂已满的目标叶子节点，并插入记录
  cursor: 指向将要分裂的叶子节点；
  key：新插入的cell 的 key；
  value：新插入的 Row；
*/
void leaf_node_split_and_insert(Cursor* cursor, uint32_t key, Row* value);


uint32_t* internal_node_num_keys(void* node);

uint32_t* internal_node_right_child(void* node);

uint32_t* internal_node_cell(void* node, uint32_t cell_num);

uint32_t* internal_node_child(void* node, uint32_t child_num);

uint32_t* internal_node_key(void* node, uint32_t key_num);

uint32_t internal_node_find_child(void* node, uint32_t key);

Cursor* internal_node_find(Table* table, uint32_t page_num, uint32_t key);

void internal_node_insert(Table* table, uint32_t parent_page_num, uint32_t child_page_num);

// 将 old_key -> new_key
void update_internal_node_key(void* node, uint32_t old_key, uint32_t new_key);


uint32_t get_node_max_key(void* node);

// 可视化
void print_tree(Pager* pager, uint32_t page_num, uint32_t indentation_level);

void indent(uint32_t level);
#endif