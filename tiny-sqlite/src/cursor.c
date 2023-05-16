#include "../include/cursor.h"
#include "../include/tree_node.h"

Cursor *table_start(Table *table)
{
  Cursor* cursor = table_find(table, 0);
  void* node = get_page(table->pager, cursor->page_num);
  uint32_t num_cells = *leaf_node_num_cells(node);
  cursor->end_of_table = (num_cells == 0);
  return cursor;
}

Cursor *table_end(Table *table)
{
  Cursor *cursor = (Cursor *)malloc(sizeof(Cursor));
  cursor->table = table;
  cursor->page_num = table->root_page_num;

  void *root_node = get_page(table->pager, table->root_page_num);
  uint32_t num_cells = *leaf_node_num_cells(root_node);
  cursor->cell_num = num_cells;
  cursor->end_of_table = true;
  return cursor;
}

Cursor *table_find(Table *table, uint32_t key_to_insert)
{
  uint32_t root_page_num = table->root_page_num;
  void *root_node = get_page(table->pager, root_page_num);
  if (get_node_type(root_node) == NODE_LEAF)
  {
    return leaf_node_find(table, root_page_num, key_to_insert);
  }
  else
  {
    return internal_node_find(table, root_page_num, key_to_insert);
  }
}

void cursor_advance(Cursor *cursor)
{
  uint32_t page_num = cursor->page_num;
  void *node = get_page(cursor->table->pager, page_num);
  cursor->cell_num += 1;
  if (cursor->cell_num >= (*leaf_node_num_cells(node)))
  {
    uint32_t next_page_num = *leaf_node_next_leaf(node);
    if (next_page_num == 0) {
      // 为该叶子节点的最后一个cell
      cursor->end_of_table = true;
    } else {
      cursor->page_num = next_page_num;
      cursor->cell_num = 0;
    }
  }
}

void *cursor_value(Cursor *cursor)
{
  void *page = get_page(cursor->table->pager, cursor->page_num);
  return leaf_node_value(page, cursor->cell_num);
}