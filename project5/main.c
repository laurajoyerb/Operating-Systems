#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include "fs.h"
#include "disk.h"

// void testcase7a(int desc) {
//   fs_lseek(desc, offset);
// }

int main() {
  char* disk_name = "mydisk";
  char* name1 = "name1";
  char* name2 = "newfilename1";
  char* name3 = "openfile1";
  size_t nbyte = 10;
  void* buf[10];
  char write_buf[10] = "123456789"; //malloc(sizeof(void*) * 4096);
  // memcpy(write_buf, "hey", nbyte);
  // char** files; // = NULL; //malloc(sizeof(char) * 15 * 64);
  off_t offset = 0;
  // off_t length = 15;

  // make_fs(disk_name);
  mount_fs(disk_name);
  fs_create(name1);
  fs_create(name2);
  fs_create(name3);

  int foo1 = fs_open(name1);
  int foo2 = fs_open(name2);
  int foo3 = fs_open(name3);

  fs_lseek(foo1, offset);

  fs_write(foo1, (void *) write_buf, nbyte);

  fs_lseek(foo1, offset);

  fs_read(foo1, buf, nbyte);
  printf("Read: %s\n", (char*)buf);

  fs_close(foo1);
  fs_close(foo2);
  fs_close(foo3);

  // fs_truncate(foo1, length);

  // printf("Files are:\n");
  // fs_listfiles(&files);

  umount_fs(disk_name);
  //
  // fs_delete(name);
  // fs_get_filesize(fildes);

  return 0;
}
