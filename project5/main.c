#include<stdio.h>
#include<stdlib.h>
#include "fs.h"
#include "disk.h"

int main() {
  char* disk_name = "helloooo";
  char* name = "name";
  int fildes = 10;
  void* buf = NULL;
  size_t nbyte = 10;
  char*** files = NULL;
  off_t offset = 10;
  off_t length = 10;

  make_fs(disk_name);
  mount_fs(disk_name);
  umount_fs(disk_name);

  fs_open(name);
  fs_close(fildes);
  fs_create(name);
  fs_delete(name);
  fs_read(fildes, buf, nbyte);
  fs_write(fildes, buf, nbyte);
  fs_get_filesize(fildes);
  fs_listfiles(files);
  fs_lseek(fildes, offset);
  fs_truncate(fildes, length);

  return 0;
}
