#include<stdio.h>
#include<stdlib.h>
#include "fs.h"
#include "disk.h"

int main() {
  char* disk_name = "mydisk";
  // char* name = "name";
  // int fildes = 10;
  // void* buf = NULL;
  // size_t nbyte = 10;
  // char*** files = NULL;
  // off_t offset = 10;
  // off_t length = 10;

  // char* write_buf = "writing";
  char read_buf[20] = {0};

  // printf("Return values:\n");
  // int make = make_disk(disk_name);
  // printf("\tMake: %d\n", make);
  // int open = open_disk(disk_name);
  // printf("\tOpen: %d\n", open);
  // int write = block_write(1, "heyy");
  // printf("\tWrite: %d\n", write);
  // int read = block_read(1, read_buf);
  // printf("\tRead: %d\n", read);
  //
  // printf("Block read gave us: %c%c%c\n", read_buf[0], read_buf[1], read_buf[2]);
  //
  // int close = close_disk(disk_name);
  // printf("\tClose: %d\n", close);

  make_fs(disk_name);
  open_disk(disk_name);
  block_read(1, read_buf);
  printf("Block read gave us: %s\n", read_buf);
  close_disk(disk_name);
  // mount_fs(disk_name);
  // umount_fs(disk_name);
  //
  // fs_open(name);
  // fs_close(fildes);
  // fs_create(name);
  // fs_delete(name);
  // fs_read(fildes, buf, nbyte);
  // fs_write(fildes, buf, nbyte);
  // fs_get_filesize(fildes);
  // fs_listfiles(files);
  // fs_lseek(fildes, offset);
  // fs_truncate(fildes, length);

  return 0;
}
