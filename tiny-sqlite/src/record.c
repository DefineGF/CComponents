#include "../include/record.h"

void serialize_row(Row* source, void* destination) {
  memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
  strncpy(destination + USERNAME_OFFSET, (source->username), USERNAME_SIZE);
  strncpy(destination + EMAIL_OFFSET, (source->email), EMAIL_SIZE);
}

void deserialize_row(void* source, Row* destination) {
  memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
  strncpy((destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
  strncpy((destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

void print_row(Row* row) {
  printf("id: %d\t usrname: %s\t email: %s\n", row->id, row->username, row->email);
}