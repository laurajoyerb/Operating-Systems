#include<stdio.h>
#include<stdlib.h>
#include "disk.h"

#define MAX_F_NAME 15
#define MAX_FILDES 32

struct super_block {
  int fat_idx; // first block of FAT
  int fat_len; // length of FAT in blocks
  int dir_idx; // first block of directory
  int dir_len; // length of directory in blocks
  int data_idx; // first block of file-data
};

struct dir_entry {
  int used; // Is this file-”slot” in use
  char name [MAX_F_NAME + 1]; // DOH!
  int size; // file size
  int head; // first data block of file
  int ref_cnt;
  // how many open file descriptors are there?
  // ref_cnt > 0 -> cannot delete file
};

struct file_descriptor {
  int used; // fd in use
  int file; // the first block of the file
  // (f) to which fd refers too
  int offset; // position of fd within f
};

struct super_block fs;
struct file_descriptor fildes[MAX_FILDES]; // 32
int *FAT; // Will be populated with the FAT data
struct dir_entry *DIR; // Will be populated with
//the directory data

int make_fs(char *disk_name) {
  return 0;
}

int mount_fs(char *disk_name) {
  return 0;
}

int umount_fs(char *disk_name) {
  return 0;
}

int fs_open(char *name) {
  return 0;
}

int fs_close(int fildes) {
  return 0;
}

int fs_create(char *name) {
  return 0;
}

int fs_delete(char *name) {
  return 0;
}

int fs_read(int fildes, void *buf, size_t nbyte) {
  return 0;
}

int fs_write(int fildes, void *buf, size_t nbyte) {
  return 0;
}

int fs_get_filesize(int fildes) {
  return 0;
}

int fs_listfiles(char ***files) {
  return 0;
}

int fs_lseek(int fildes, off_t offset) {
  return 0;
}

int fs_truncate(int fildes, off_t length) {
  return 0;
}
