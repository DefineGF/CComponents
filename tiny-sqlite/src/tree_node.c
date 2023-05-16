#include "../include/tree_node.h"


NodeType get_node_type(void *node)
{
  uint8_t value = *((uint8_t *)(node + NODE_TYPE_OFFSET));
  return (NodeType)value;
}

bool is_node_root(void *node)
{
  uint8_t value = *((uint8_t *)(node + IS_ROOT_OFFSET));
  return (bool)value;
}

void set_node_is_root(void *node, bool is_root)
{
  *((uint8_t *)(node + IS_ROOT_OFFSET)) = (uint8_t)is_root;
}

uint32_t* node_parent(void* node) {
  return node + PARENT_POINTER_OFFSET;
}

/*
  需要创建新的根节点：
  1. 首先获取原先的已经分裂的 root 节点；
  2. 重新申请一个空白 page， 命名为 left_child;
  3. 将原先 root 节点内容复制到 left_child 中；
  4. 设置各类参数！
*/
void create_new_root(Table *table, uint32_t right_page_num)
{
  void *root = get_page(table->pager, table->root_page_num);
  void *right_child = get_page(table->pager, right_page_num);

  uint32_t left_child_page_num = get_unused_page_num(table->pager);
  void *left_child = get_page(table->pager, left_child_page_num);

  // 申请空白的页面 left_child，并将原root页面中的内容复制给 left_child
  // 原 root 已经分裂出了 right_page_num
  memcpy(left_child, root, PAGE_SIZE);
  set_node_is_root(left_child, false);

  // 更新 root 节点信息
  initial_internal_node(root);
  set_node_is_root(root, true);
  *internal_node_num_keys(root) = 1;
  *internal_node_child(root, 0) = left_child_page_num;        // 设置首个cell 的 child 页号
  *internal_node_key(root, 0) = get_node_max_key(left_child); // 设置首个cell 的 key 值 = 左叶子上最大值
  *internal_node_right_child(root) = right_page_num;          // 设置右侧 child 的页号
  *node_parent(left_child) = table->root_page_num;
  *node_parent(right_child) = table->root_page_num;
}

void set_node_type(void *node, NodeType type)
{
  uint8_t value = (uint8_t)type;
  *((uint8_t *)(node + NODE_TYPE_OFFSET)) = value;
}

uint32_t *leaf_node_num_cells(void *node)
{
  return node + LEAF_NODE_NUM_CELLS_OFFSET; // 偏移公共 Header 长度
}

// cell_num 从0开始, 返回cell_num 对应的 cell 地址
void *leaf_node_cell(void *node, uint32_t cell_num)
{
  return node + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE;
}

uint32_t *leaf_node_key(void *node, uint32_t cell_num)
{
  return leaf_node_cell(node, cell_num);
}

uint32_t *leaf_node_next_leaf(void *node)
{
  return node + LEAF_NODE_NEXT_LEAF_OFFSET;
}

void *leaf_node_value(void *node, uint32_t cell_num)
{
  return leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
}

void initial_leaf_node(void *node)
{
  set_node_type(node, NODE_LEAF); // 设置 叶子类型
  set_node_is_root(node, false);
  *leaf_node_num_cells(node) = 0; // 设置 cell 个数: 0
  *leaf_node_next_leaf(node) = 0;
}

void initial_internal_node(void *node)
{
  set_node_type(node, NODE_INTERNAL);
  set_node_is_root(node, false);
  *leaf_node_num_cells(node) = 0;
}

Cursor *leaf_node_find(Table *table, uint32_t page_num, uint32_t key)
{
  void *node = get_page(table->pager, page_num);
  uint32_t num_cells = *leaf_node_num_cells(node);

  Cursor *cursor = (Cursor *)malloc(sizeof(Cursor));
  cursor->table = table;
  cursor->page_num = page_num;
  cursor->end_of_table = false;

  uint32_t left_index = 0, right_index = num_cells;
  while (left_index != right_index)
  {
    uint32_t mid_index = (left_index + right_index) / 2;
    uint32_t key_mid = *leaf_node_key(node, mid_index);
    if (key == key_mid)
    {
      cursor->cell_num = mid_index;
      return cursor;
    }
    if (key < key_mid)
    {
      right_index = mid_index;
    }
    else
    {
      left_index = mid_index + 1;
    }
  }
  cursor->cell_num = left_index;
  return cursor;
}

void leaf_node_insert(Cursor *cursor, uint32_t key, Row *value)
{
  void *node = get_page(cursor->table->pager, cursor->page_num);
  uint32_t num_cells = *leaf_node_num_cells(node);
  if (num_cells >= LEAF_NODE_MAX_CELLS)
  {
    // 超过该节点容纳的最大数
    leaf_node_split_and_insert(cursor, key, value);
    return;
  }
  if (cursor->cell_num < num_cells)
  {
    // 向后移动一个 cell
    for (uint32_t i = num_cells; i > cursor->cell_num; i--)
    {
      memcpy(leaf_node_cell(node, i), leaf_node_cell(node, i - 1), LEAF_NODE_CELL_SIZE);
    }
  }
  *(leaf_node_num_cells(node)) += 1;
  *(leaf_node_key(node, cursor->cell_num)) = key;
  serialize_row(value, leaf_node_value(node, cursor->cell_num));
}

/*
  old_page 分裂出 new_page:
              5
             / \
  {1:v1, 5:v5}  {12:v12}
              ||

               3，5
            /   |    \
{1:v1, 3:v3}   {5:v5}  {12:v12}
*/
void leaf_node_split_and_insert(Cursor *cursor, uint32_t key, Row *value)
{
  void *old_node = get_page(cursor->table->pager, cursor->page_num);
  uint32_t old_max = get_node_max_key(old_node);

  // 获取首个未被使用的 page 索引
  uint32_t new_page_num = get_unused_page_num(cursor->table->pager); 
  void *new_node = get_page(cursor->table->pager, new_page_num);
  initial_leaf_node(new_node);
  *node_parent(new_node) = *node_parent(old_node); // 为分裂出的新页设置同一父节点
  *leaf_node_next_leaf(new_node) = *leaf_node_next_leaf(old_node);
  *leaf_node_next_leaf(old_node) = new_page_num;

  /*
    LEAF_NODE_MAX_CELLS = 13
    当 cursor->cell_num = 13 时，0-6 会映射到 old_node; 7-13 会映射到 new_node上, i = [0, 6];

  */
  for (int32_t i = LEAF_NODE_MAX_CELLS; i >= 0; i--)
  {
    void *destination_node = (i >= LEAF_NODE_LEFT_SPLIT_COUNT) ? new_node : old_node;

    uint32_t index = i % LEAF_NODE_LEFT_SPLIT_COUNT; // 写入新页的 cell 索引
    void *des_cell_addr = leaf_node_cell(destination_node, index);

    printf("cur i = %d\t, cursor->cell_num = %d\t, index = %d\n", i, cursor->cell_num, index);

    if (i == cursor->cell_num)
    {
      serialize_row(value, leaf_node_value(destination_node, index));
      *leaf_node_key(destination_node, index) = key;
    }
    else if (i > cursor->cell_num)
    {
      // 将目标cell_num 前一个 cell 移动到 des_cell_addr 位置（便于将目标cell写入）
      memcpy(des_cell_addr, leaf_node_cell(old_node, i - 1), LEAF_NODE_CELL_SIZE);
    }
    else
    {
      // 如果是目标cell_num 之前的，则原封不动复制到相应的位置
      memcpy(des_cell_addr, leaf_node_cell(old_node, i), LEAF_NODE_CELL_SIZE);
    }
  }
  // 更新 新 & 旧 页的cell 数
  *(leaf_node_num_cells(old_node)) = (uint32_t)LEAF_NODE_LEFT_SPLIT_COUNT;
  *(leaf_node_num_cells(new_node)) = (uint32_t)LEAF_NODE_RIGHT_SPLIT_COUNT;

  if (is_node_root(old_node))
  {
    // 当前节点为根节点时，由于分裂，需要创建新的根节点
    return create_new_root(cursor->table, new_page_num);
  }
  else
  {
    // 当前节点非根节点
    uint32_t parent_page_num = *node_parent(old_node); 
    uint32_t new_max = get_node_max_key(old_node); // 这里最大key是根据 cell 数计算出来的
    void* parent = get_page(cursor->table->pager, parent_page_num);

    // 更新 父节点的 key
    update_internal_node_key(parent, old_max, new_max);
    internal_node_insert(cursor->table, parent_page_num, new_page_num);
    exit(EXIT_FAILURE);
  }
}

void print_leaf_node(void *node)
{
  uint32_t num_cells = *leaf_node_num_cells(node);
  printf("leaf cells count: %d\n", num_cells);

  for (uint32_t i = 0; i < num_cells; ++i)
  {
    uint32_t key = *leaf_node_key(node, i);
    printf("\tkey = %d\n", key);
  }
}

// 内部节点 key 个数 起始地址
uint32_t *internal_node_num_keys(void *node)
{
  return node + INTERNAL_NODE_NUM_KEYS_OFFSET;
}

// 内部节点 child 个数 起始地址
uint32_t *internal_node_right_child(void *node)
{
  return node + INTERNAL_NODE_RIGHT_CHILD_OFFSET;
}

// 内部节点指定 cell_num 的地址
uint32_t *internal_node_cell(void *node, uint32_t cell_num)
{
  return node + INTERNAL_NODE_HEADER_SIZE + cell_num * INTERNAL_NODE_CELL_SIZE;
}

// 根据 child_num 获取内部节点中相应 child 的指针
uint32_t *internal_node_child(void *node, uint32_t child_num)
{
  uint32_t num_keys = *internal_node_num_keys(node);
  if (child_num > num_keys)
  {
    printf("Tried to access child_num %d > num_keys %d\n", child_num, num_keys);
    exit(EXIT_FAILURE);
  }
  else if (child_num == num_keys)
  {
    return internal_node_right_child(node);
  }
  else
  {
    return internal_node_cell(node, child_num);
  }
}

uint32_t*internal_node_key(void *node, uint32_t key_num)
{
  return (void*)internal_node_cell(node, key_num) + INTERNAL_NODE_CHILD_SIZE;
}

uint32_t internal_node_find_child(void* node, uint32_t key) 
{
  uint32_t num_keys = *internal_node_num_keys(node);
  uint32_t left_index = 0, right_index = num_keys;
  while (left_index != right_index)
  {
    uint32_t index = (left_index + right_index) / 2;
    uint32_t key_index = *internal_node_key(node, index);
    if (key_index >= key)
    {
      right_index = index;
    }
    else
    {
      left_index = index + 1;
    }
  }
  return left_index;
}

Cursor *internal_node_find(Table *table, uint32_t page_num, uint32_t key)
{
  void* node = get_page(table->pager, page_num);
  uint32_t child_index = internal_node_find_child(node, key);
  uint32_t child_page_num = *internal_node_child(node, child_index);
  void* child = get_page(table->pager, child_page_num);
  switch (get_node_type(child)) {
    case NODE_LEAF:
      return leaf_node_find(table, child_page_num, key);
    case NODE_INTERNAL:
      return internal_node_find(table, child_page_num, key);
  }
}

void internal_node_insert(Table* table, uint32_t parent_page_num, uint32_t child_page_num) {
  void* parent = get_page(table->pager, parent_page_num);
  void* child = get_page(table->pager, child_page_num);
  uint32_t child_max_key = get_node_max_key(child);
  uint32_t index = internal_node_find_child(parent, child_max_key);

  uint32_t original_num_keys = *internal_node_num_keys(parent);
  *internal_node_num_keys(parent) = original_num_keys + 1;

  if (original_num_keys >= INTERNAL_NODE_MAX_CELLS) {
    printf("Need to implement splitting internal node\n");
    exit(EXIT_FAILURE);
  }

  uint32_t right_child_page_num = *internal_node_right_child(parent);
  void* right_child = get_page(table->pager, right_child_page_num);

  if (child_max_key > get_node_max_key(right_child)) {
    /* Replace right child */
    *internal_node_child(parent, original_num_keys) = right_child_page_num;
    *internal_node_key(parent, original_num_keys) = get_node_max_key(right_child);
    *internal_node_right_child(parent) = child_page_num;
  } else {
    /* Make room for the new cell */
    for (uint32_t i = original_num_keys; i > index; i--) {
      void* destination = internal_node_cell(parent, i);
      void* source = internal_node_cell(parent, i - 1);
      memcpy(destination, source, INTERNAL_NODE_CELL_SIZE);
    }
    *internal_node_child(parent, index) = child_page_num;
    *internal_node_key(parent, index) = child_max_key;
  }
}

void update_internal_node_key(void* node, uint32_t old_key, uint32_t new_key) 
{
  uint32_t old_child_index = internal_node_find_child(node, old_key);
  *internal_node_key(node, old_child_index) = new_key;
}

uint32_t get_node_max_key(void *node)
{
  switch (get_node_type(node))
  {
  case NODE_INTERNAL:
    return *internal_node_key(node, *internal_node_num_keys(node) - 1);
  case NODE_LEAF:
    return *leaf_node_key(node, *leaf_node_num_cells(node) - 1);
  }
}

void print_tree(Pager *pager, uint32_t page_num, uint32_t indentation_level)
{
  void *node = get_page(pager, page_num);
  uint32_t num_keys, child;
  switch (get_node_type(node))
  {
  case (NODE_LEAF):
    num_keys = *leaf_node_num_cells(node);
    indent(indentation_level);
    printf("- leaf (size %d)\n", num_keys);
    for (uint32_t i = 0; i < num_keys; i++)
    {
      indent(indentation_level + 1);
      printf("- %d\n", *leaf_node_key(node, i));
    }
    break;
  case (NODE_INTERNAL):
    num_keys = *internal_node_num_keys(node);
    indent(indentation_level);
    printf("- internal (size %d)\n", num_keys);
    for (uint32_t i = 0; i < num_keys; i++)
    {
      child = *internal_node_child(node, i);
      print_tree(pager, child, indentation_level + 1);

      indent(indentation_level + 1);
      printf("- key %d\n", *internal_node_key(node, i));
    }
    child = *internal_node_right_child(node);
    print_tree(pager, child, indentation_level + 1);
    break;
  }
}

void indent(uint32_t level)
{
  for (uint32_t i = 0; i < level; i++)
  {
    printf("  ");
  }
}