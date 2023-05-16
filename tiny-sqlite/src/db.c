#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "../include/record.h"
#include "../include/table.h"
#include "../include/config.h"
#include "../include/cursor.h"
#include "../include/tree_node.h"

typedef struct
{
  char *buffer;
  size_t buffer_length;
  ssize_t input_length;
} InputBuffer;

typedef enum
{
  META_COMMAND_SUCCESS,
  META_COMMAND_UNRECOGNIZED,
} MetaCommandResult;

typedef enum
{
  PREPARE_SUCCESS,
  PREPARE_SYNTAX_ERROR,
  PREPARE_STRING_TOO_LONG,
  PREPARE_NEGATIVE_ID,
  PREPARE_UNRECOGNIZED_STATEMENT,
} PrepareResult;

typedef enum
{
  STATEMENT_INSERT,
  STATEMENT_SELECT,
} StatementType;

typedef struct
{
  StatementType type;
  Row row_to_insert;
} Statement;

typedef enum
{
  EXECUTE_DUPLICATE_KEY,
  EXECUTE_TABLE_FULL,
  EXECUTE_SUCCESS,
} ExecuteResult;

// 创建缓冲区
InputBuffer *new_input_buffer();

// 读取命令到缓冲区
void read_input(InputBuffer *input_buffer);

// 解析以 '.' 开始的元命令
MetaCommandResult do_meta_command(InputBuffer *input_buffer, Table *table);

// 解析insert、select 等命令
PrepareResult prepare_statement(InputBuffer *input_buffer, Statement *statement);

// 根据 statement-> type 执行相应的sql语句
ExecuteResult execute_statement(Statement *statement, Table *table);

// 执行插入语句
ExecuteResult execute_insert(Statement *statement, Table *table);

// 执行查询语句
ExecuteResult execute_select(Statement *statement, Table *table);

// 释放输入缓冲区
void close_input_buffer(InputBuffer *input_buffer);

InputBuffer *new_input_buffer()
{
  InputBuffer *input_buffer = (InputBuffer *)malloc(sizeof(InputBuffer));
  input_buffer->buffer = NULL;
  input_buffer->buffer_length = 0;
  input_buffer->input_length = 0;
  return input_buffer;
}

MetaCommandResult do_meta_command(InputBuffer *input_buffer, Table *table)
{
  if (strcmp(input_buffer->buffer, ".exit") == 0)
  {
    close_input_buffer(input_buffer);
    db_close(table);
    exit(EXIT_SUCCESS);
  }
  else if (strcmp(input_buffer->buffer, ".btree") == 0)
  {
    printf("tree:\n");
    print_tree(table->pager, 0, 0);
    return META_COMMAND_SUCCESS;
  }
  else
  {
    return META_COMMAND_UNRECOGNIZED;
  }
}

PrepareResult prepare_statement(InputBuffer *input_buffer, Statement *statement)
{
  if (strncmp(input_buffer->buffer, "insert", 6) == 0)
  {
    statement->type = STATEMENT_INSERT;
    int args_assinged = sscanf(input_buffer->buffer, "insert %d %s %s",
                               &statement->row_to_insert.id,
                               statement->row_to_insert.username,
                               statement->row_to_insert.email);
    if (args_assinged < 3)
    {
      return PREPARE_SYNTAX_ERROR; // 语法错误
    }
    return PREPARE_SUCCESS;
  }
  if (strcmp(input_buffer->buffer, "select") == 0)
  {
    statement->type = STATEMENT_SELECT;
    return PREPARE_SUCCESS;
  }
  return PREPARE_UNRECOGNIZED_STATEMENT;
}

ExecuteResult execute_statement(Statement *statement, Table *table)
{
  switch (statement->type)
  {
  case STATEMENT_INSERT:
    return execute_insert(statement, table);
  case STATEMENT_SELECT:
    return execute_select(statement, table);
  }
}

ExecuteResult execute_insert(Statement *statement, Table *table)
{
  Row *row_to_insert = &(statement->row_to_insert);
  uint32_t key_to_insert = row_to_insert->id;
  Cursor *cursor = table_find(table, key_to_insert); // 这里的cursor可能指向首个大于 key_to_insert 的 cell

  void* node = get_page(table->pager, cursor->page_num);
  uint32_t num_cells = *leaf_node_num_cells(node);

  if (cursor->cell_num < num_cells)
  {
    // 插入的 key 在已有 cell 内部，查看是否与已有的key重复
    uint32_t target_key = *leaf_node_key(node, cursor->cell_num);
    if (target_key == key_to_insert)
    {
      return EXECUTE_DUPLICATE_KEY;
    }
  }

  leaf_node_insert(cursor, row_to_insert->id, row_to_insert);
  free(cursor); // 释放游标
  return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement *statement, Table *table)
{
  Cursor *cursor = table_start(table);
  Row row;
  while (!(cursor->end_of_table))
  {
    deserialize_row(cursor_value(cursor), &row);
    print_row(&row);
    cursor_advance(cursor);
  }
  free(cursor);
  return EXECUTE_SUCCESS;
}

void print_prompt() { printf("db > "); }

void read_input(InputBuffer *input_buffer)
{
  ssize_t bytes_read = getline(&(input_buffer->buffer),
                               &(input_buffer->buffer_length),
                               stdin);

  if (bytes_read <= 0)
  {
    ERROR("Error reading input!");
    exit(EXIT_FAILURE);
  }

  // Ignore trailing newline
  input_buffer->input_length = bytes_read - 1;
  input_buffer->buffer[bytes_read - 1] = 0;
}

void close_input_buffer(InputBuffer *input_buffer)
{
  free(input_buffer->buffer);
  free(input_buffer);
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    ERROR("pls set database file_name!");
    exit(EXIT_FAILURE);
  }
  char *file_name = argv[1];
  Table *table = db_open(file_name);

  InputBuffer *input_buffer = new_input_buffer();
  while (true)
  {
    print_prompt();
    read_input(input_buffer);

    if (input_buffer->buffer[0] == '.')
    {
      switch (do_meta_command(input_buffer, table))
      {
      case META_COMMAND_SUCCESS:
        continue;

      case META_COMMAND_UNRECOGNIZED:
        printf("unrecognized command: %s!\n", input_buffer->buffer);
        continue;
      }
    }

    Statement statement;
    switch (prepare_statement(input_buffer, &statement))
    {
    case PREPARE_SUCCESS: // 结束 switch，继续 execute_statement
      break;

    case PREPARE_UNRECOGNIZED_STATEMENT:
      printf("unrecognized keyword at %s \n", input_buffer->buffer);
      continue;
    case PREPARE_SYNTAX_ERROR:
      printf("syntax_error: %s\n", input_buffer->buffer);
      continue;
    }

    switch (execute_statement(&statement, table))
    {
    case EXECUTE_SUCCESS:
      printf("executed!\n");
      break;
    case EXECUTE_TABLE_FULL:
      printf("error: table full!\n");
      break;
    case EXECUTE_DUPLICATE_KEY:
      printf("error: duplicate key!\n");
      break;
    }
  }
}