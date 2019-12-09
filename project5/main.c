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
  char*** files = NULL; //malloc(sizeof(char) * 15 * 64);
  // off_t offset = 10;
  // off_t length = 10;

  // make_fs(disk_name);
  mount_fs(disk_name);
  // fs_create(name);
  int foo1 = fs_open(name1);
  int foo2 = fs_open(name1);
  int foo3 = fs_open(name1);

  fs_close(foo1);
  fs_close(foo2);
  fs_close(foo3);

  printf("Files are:\n");
  fs_listfiles(files);

  umount_fs(disk_name);
  //
  // fs_close(fildes);
  // fs_create(name);
  // fs_delete(name);
  // fs_read(fildes, buf, nbyte);
  // fs_write(fildes, buf, nbyte);
  // fs_get_filesize(fildes);
  // printf("File 1: %s\n", *files[0]);
  // fs_lseek(fildes, offset);
  // fs_truncate(fildes, length);

  return 0;
}
