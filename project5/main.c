#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include "fs.h"
#include "disk.h"

int main() {
  char* disk_name = "mydisk";
  char* name1 = "name1";
  // char* name2 = "newfilename1";
  // char* name3 = "openfile1";
  size_t nbyte = 10;
  void* buf = malloc(sizeof(void*) * 4096);
  void* write_buf = malloc(sizeof(void*) * 4096);
  memcpy(write_buf, "hey", nbyte);
  // char** files; // = NULL; //malloc(sizeof(char) * 15 * 64);
  // off_t offset = 10;
  // off_t length = 10;

  // make_fs(disk_name);
  mount_fs(disk_name);
  // fs_create(name1);
  // fs_create(name2);
  // fs_create(name3);
  int foo1 = fs_open(name1);
  // int foo2 = fs_open(name2);
  // int foo3 = fs_open(name3);
  //
  // fs_close(foo1);
  // fs_close(foo2);
  // fs_close(foo3);

  // printf("Files are:\n");
  // fs_listfiles(&files);
  // int num = 0;
  // while(files[num++] != NULL) {
  // };
  // printf("Num of files is: %d\n", num - 1);
  fs_write(foo1, buf, nbyte);
  fs_read(foo1, buf, nbyte);

  umount_fs(disk_name);
  //
  // fs_close(fildes);
  // fs_create(name);
  // fs_delete(name);
  // fs_read(fildes, buf, nbyte);
  // fs_get_filesize(fildes);
  // printf("File 1: %s\n", *files[0]);
  // fs_lseek(fildes, offset);
  // fs_truncate(fildes, length);

  return 0;
}
