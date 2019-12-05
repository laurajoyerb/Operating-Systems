#include<stdio.h>
#include<stdlib.h>
#include "fs.h"
#include "disk.h"

int main() {
  char* disk_name = "mydisk";
  char* name1 = "name";
  // char* name2 = "newfilename";
  // char* name3 = "openthisfile";
  // void* buf = NULL;
  // size_t nbyte = 10;
  // char*** files = NULL;
  // off_t offset = 10;
  // off_t length = 10;

  // make_fs(disk_name);
  mount_fs(disk_name);
  // fs_create(name);
  int foo1 = fs_open(name1);
  printf("File descriptor is: %d for file %s\n", foo1, name1);
  int foo2 = fs_open(name1);
  printf("File descriptor is: %d for file %s\n", foo2, name1);
  int foo3 = fs_open(name1);
  printf("File descriptor is: %d for file %s\n", foo3, name1);

  fs_close(foo1);
  printf("File descriptor %d has been closed\n", foo1);
  fs_close(foo2);
  printf("File descriptor %d has been closed\n", foo2);
  fs_close(foo3);
  printf("File descriptor %d has been closed\n", foo3);


  umount_fs(disk_name);
  //
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

