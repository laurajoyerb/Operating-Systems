#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "disk.h"

#define MAX_F_NAME 15
#define MAX_FILDES 64
#define SUPER_BLOCK_NUM 0

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

struct super_block* fs;
struct file_descriptor fildes[MAX_FILDES]; // 32
int* FAT; // Will be populated with the FAT data
struct dir_entry* DIR; // Will be populated with
//the directory data

int make_fs(char *disk_name) {
  if (make_disk(disk_name) < 0) {
    return -1;
  }

  if (open_disk(disk_name) < 0) {
    return -1;
  }

  fs = calloc(1, sizeof(struct super_block));
  FAT = calloc(4096, sizeof(int));
  DIR = calloc(64, sizeof(struct dir_entry));

  fs->fat_idx = 1;
  fs->fat_len = 0;
  fs->dir_idx = 2;
  fs->dir_len = 1;
  fs->data_idx = 3;

  int i;
  for (i = 0; i < 64; i++) {
    DIR[i].used = false;
  }

  // reserved blocks
  FAT[0] = -3;
  FAT[1] = -3;
  FAT[2] = -3;

  for (i = 3; i < 4096; i++) {
    FAT[i] = -1; // free
  }

  if (block_write(0, (char*) fs) < 0) {
    return -1;
  }

  printf("Wrote super block\n");

  if (block_write(1, (char*) FAT) < 0) {
    return -1;
  }

  printf("Wrote FAT\n");

  if (block_write(2, (char*) DIR) < 0) {
    return -1;
  }

  printf("Wrote DIR\n");

  if (close_disk(disk_name) < 0) {
    return -1;
  }

  return 0;
}

int mount_fs(char *disk_name) {

  if (open_disk(disk_name) < 0) {
    return -1;
  }

  fs = calloc(1, sizeof(struct super_block));
  FAT = calloc(4096, sizeof(int));
  DIR = calloc(64, sizeof(struct dir_entry));

  if (block_read(0, (char*) fs) < 0) {
    return -1;
  }

  printf("Read superblock\n");

  if (block_read(0, (char*) FAT) < 0) {
    return -1;
  }

  printf("Read FAT\n");

  if (block_read(0, (char*) DIR) < 0) {
    return -1;
  }

  printf("Read DIR\n");

  return 0;
}

int umount_fs(char *disk_name) {

  if (close_disk(disk_name) < 0) {
    return -1;
  }

  return 0;
}

int fs_open(char *name) {
  return 0;
}

int fs_close(int fildes) {
  return 0;
}

int fs_create(char *name) {
  if (strlen(name) > 15) {
    printf("Error: File name too long\n");
    return -1;
  }


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
