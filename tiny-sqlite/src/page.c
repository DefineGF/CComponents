#include "../include/page.h"

Pager *pager_open(const char *file_name)
{
  int fd = open(file_name,
                O_RDWR | O_CREAT, // 读写模式 + 没有则创建
                S_IWUSR | S_IRUSR // 用户写权限 | 读权限
  );
  if (fd == -1)
  {
    printf("error: unable to open file!\n");
    exit(EXIT_FAILURE);
  }
  off_t file_length = lseek(fd, 0, SEEK_END);

  Pager *pager = (Pager *)malloc(sizeof(Pager));
  pager->file_descirptor = fd;
  pager->file_len = file_length;
  pager->num_pages = (file_length / PAGE_SIZE);

  if ((file_length % PAGE_SIZE) != 0)
  {
    printf("error: db file format error!\n");
    exit(EXIT_FAILURE);
  }

  for (uint32_t i = 0; i < TABLE_MAX_PAGES; ++i)
  {
    pager->pages[i] = NULL;
  }
  return pager;
}

void *get_page(Pager *pager, uint32_t page_num)
{
  if (page_num >= TABLE_MAX_PAGES)
  {
    printf("error: page_num = %d out of max_page_counts = %d\n", page_num, TABLE_MAX_PAGES);
    exit(EXIT_FAILURE);
  }
  if (pager->pages[page_num] == NULL)
  {
    void *page = malloc(PAGE_SIZE);
    DEBUG("malloc a new page!");

    uint32_t num_pages = pager->file_len / PAGE_SIZE;
    if (pager->file_len % PAGE_SIZE)
    {
      num_pages += 1;
    }

    // 若该页 持久化 在磁盘上，则从磁盘读取
    if (page_num <= num_pages)
    {
      lseek(pager->file_descirptor, page_num * PAGE_SIZE, SEEK_SET);
      ssize_t bytes_read = read(pager->file_descirptor, page, PAGE_SIZE);
      if (bytes_read == -1)
      {
        printf("error: read file failure!");
        exit(EXIT_FAILURE);
      }
    }
    pager->pages[page_num] = page;

    if (page_num >= pager->num_pages)
    {
      pager->num_pages = page_num + 1;
    }
  }
  return pager->pages[page_num];
}

uint32_t get_unused_page_num(Pager *pager)
{
  return pager->num_pages;
}

void pager_flush(Pager *pager, uint32_t page_num)
{
  if (pager->pages[page_num] == NULL)
  {
    ERROR("flush null page!");
    exit(EXIT_FAILURE);
  }
  off_t offset = lseek(pager->file_descirptor,
                       page_num * PAGE_SIZE, SEEK_CUR);
  if (offset == -1)
  {
    ERROR("seek error!");
    exit(EXIT_FAILURE);
  }
  ssize_t bytes_written = write(pager->file_descirptor,
                                pager->pages[page_num],
                                PAGE_SIZE);
  if (bytes_written == -1)
  {
    ERROR("write error!");
    exit(EXIT_FAILURE);
  }
}