#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include "fs.h"
#include "disk.h"

int main() {
  char* disk_name = "mydisk";
  char* name1 = "name1";
  char* name2 = "newfilename1";
  char* name3 = "openfile1";
  size_t nbyte = 10;
  void* buf[10];
  char write_buf[10] = "123456789";
  char** files;
  off_t offset = 0;
  int i;

  // make_fs(disk_name);
  mount_fs(disk_name);

  // fs_create(name1);
  // fs_create(name2);
  // fs_create(name3);

  // int foo1 = fs_open(name1);
  // int foo2 = fs_open(name2);
  // int foo3 = fs_open(name3);
  //
  // fs_lseek(foo1, offset);
  //
  // fs_write(foo1, (void *) write_buf, nbyte);
  //
  // fs_lseek(foo1, offset);
  //
  // fs_read(foo1, buf, nbyte);
  // printf("Read: %s\n", (char*)buf);
  //
  // fs_close(foo1);
  // fs_close(foo2);
  // fs_close(foo3);

  // fs_truncate(foo1, length);

  printf("Files are:\n");
  fs_listfiles(&files);

  // printf("%s %s %s\n", files[0], files[1], files[2]);
  // int i;
  // for (i = 0; i < 3; i++) {
  //   if (files[i] != NULL) {
  //     printf("%s\n", files[i]);
  //   } else {
  //     break;
  //   }
  // }

  umount_fs(disk_name);
  return 0;
}
