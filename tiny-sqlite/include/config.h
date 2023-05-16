#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>     // 对文件各种操作
#include <unistd.h>    // fork、pipe、read、write、close等

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>


#define TABLE_MAX_PAGES 64  // 页数量
#define PAGE_SIZE  4096     // 页大小


// 针对特定表设计
// id - 4; username - 32; email - 255
// total = 291
#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE    255
#define ROW_SIZE             768

#define INTERNAL_NODE_MAX_CELLS 3


// 公共: 节点类型 + 是否根节点 + 父节点指针
#define NODE_TYPE_SIZE          sizeof(uint8_t)
#define IS_ROOT_SIZE            sizeof(uint8_t)
#define PARENT_POINTER_SIZE     sizeof(uint32_t)
#define COMMON_NODE_HEADER_SIZE (NODE_TYPE_SIZE + IS_ROOT_SIZE + PARENT_POINTER_SIZE)

#define NODE_TYPE_OFFSET        0
#define IS_ROOT_OFFSET          NODE_TYPE_SIZE
#define PARENT_POINTER_OFFSET   (IS_ROOT_OFFSET + IS_ROOT_SIZE)


// 叶子头: cells个数 + 下一个叶子节点
#define LEAF_NODE_NUM_CELLS_SIZE    sizeof(uint32_t)
#define LEAF_NODE_NEXT_LEAF_SIZE    sizeof(uint32_t)

#define LEAF_NODE_NUM_CELLS_OFFSET  COMMON_NODE_HEADER_SIZE
#define LEAF_NODE_NEXT_LEAF_OFFSET  (LEAF_NODE_NUM_CELLS_OFFSET + LEAF_NODE_NUM_CELLS_SIZE)
#define LEAF_NODE_HEADER_SIZE       (COMMON_NODE_HEADER_SIZE + LEAF_NODE_NUM_CELLS_SIZE + LEAF_NODE_NEXT_LEAF_SIZE)


// 叶子body：
// 数据内容: [(key : Row), (key : Row), ...]
#define LEAF_NODE_KEY_SIZE      sizeof(uint32_t)
#define LEAF_NODE_VALUE_SIZE    ROW_SIZE
#define LEAF_NODE_CELL_SIZE     (LEAF_NODE_KEY_SIZE + LEAF_NODE_VALUE_SIZE)

#define LEAF_NODE_KEY_OFFSET        0
#define LEAF_NODE_VALUE_OFFSET      (LEAF_NODE_KEY_OFFSET + LEAF_NODE_KEY_SIZE)
#define LEAF_NODE_SPACE_FOR_CELLS   (PAGE_SIZE - LEAF_NODE_HEADER_SIZE)
#define LEAF_NODE_MAX_CELLS         ((LEAF_NODE_SPACE_FOR_CELLS) / (LEAF_NODE_CELL_SIZE))

// 节点分裂
#define LEAF_NODE_RIGHT_SPLIT_COUNT  ((LEAF_NODE_MAX_CELLS + 1) / 2)
#define LEAF_NODE_LEFT_SPLIT_COUNT   ((LEAF_NODE_MAX_CELLS + 1) - LEAF_NODE_RIGHT_SPLIT_COUNT)

// 内部节点 header：key 个数 + right child pointer
#define INTERNAL_NODE_NUM_KEYS_SIZE     sizeof(uint32_t)
#define INTERNAL_NODE_RIGHT_CHILD_SIZE  sizeof(uint32_t)

#define INTERNAL_NODE_NUM_KEYS_OFFSET     COMMON_NODE_HEADER_SIZE
#define INTERNAL_NODE_RIGHT_CHILD_OFFSET  (INTERNAL_NODE_NUM_KEYS_OFFSET + INTERNAL_NODE_NUM_KEYS_SIZE)
#define INTERNAL_NODE_HEADER_SIZE         (COMMON_NODE_HEADER_SIZE + INTERNAL_NODE_NUM_KEYS_SIZE + INTERNAL_NODE_RIGHT_CHILD_SIZE)

// 内部节点 body: child + key
#define INTERNAL_NODE_KEY_SIZE      sizeof(uint32_t)
#define INTERNAL_NODE_CHILD_SIZE    sizeof(uint32_t)
#define INTERNAL_NODE_CELL_SIZE     (INTERNAL_NODE_CHILD_SIZE + INTERNAL_NODE_KEY_SIZE)

#define DEBUG(msg) printf("debug: %s\n", msg)
#define DEBUGS(format, args...) printf("debug: ", args)

#define ERROR(msg) printf("error: %s\n", msg)
#endif