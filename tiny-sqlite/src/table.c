#include "../include/table.h"
#include "../include/tree_node.h"

Table* db_open(const char* file_name) {
    Pager* pager = pager_open(file_name);
    
    Table *table = (Table*)malloc(sizeof(Table));
    table->pager = pager;
    table->root_page_num = 0;
    
    if (pager->num_pages == 0) {
        void* root_node = get_page(pager, 0);
        initial_leaf_node(root_node);
        set_node_is_root(root_node, true);
    }
    return table;
}

void db_close(Table* table) {
    Pager* pager = table->pager;
   
    for (u_int32_t i = 0; i < pager->num_pages; ++i) {
        if (pager->pages[i] == NULL) continue;

        pager_flush(pager, i);
        free(pager->pages[i]);
        pager->pages[i] = NULL;
    }


    int result = close(pager->file_descirptor);
    if (result == -1) {
        ERROR("close file error!");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; ++i) {
        void* page = pager->pages[i];
        if (page) {
            free(page);
            pager->pages[i] = NULL;
        }
    }
    free(pager);
    free(table);
}

void *row_slot(Table* table, uint32_t row_num) {
    const uint32_t row_count_per_page = PAGE_SIZE / ROW_SIZE;

    uint32_t page_num = row_num / row_count_per_page;
    void *page = get_page(table->pager, page_num);

    uint32_t row_offset = row_num % row_count_per_page;
    uint32_t byte_offset = row_offset * ROW_SIZE; // 该行所在的字节偏移
    return page + byte_offset;
}

