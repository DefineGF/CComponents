#ifndef _RECORD_H_
#define _RECORD_H_

#include "config.h"

#define size_of_attribute(_struct, _attribute) sizeof(((_struct*)0)->_attribute)

typedef struct {
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];
} Row;

#define ID_SIZE         4
#define USERNAME_SIZE   32
#define EMAIL_SIZE      255

#define ID_OFFSET       0
#define USERNAME_OFFSET 4
#define EMAIL_OFFSET    259

// 数据库行记录序列化
void serialize_row(Row *source, void *destination);

// 反序列化
void deserialize_row(void *source, Row *destination);

void print_row(Row* row);
#endif