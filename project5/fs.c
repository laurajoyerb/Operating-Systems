#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "disk.h"

#define MAX_F_NAME 15
#define MAX_FILDES 32
#define SUPER_BLOCK_NUM 0

struct super_block {
  int fat_idx; // first block of FAT
  int fat_len; // length of FAT in blocks
  int dir_idx; // first block of directory
  int dir_len; // length of directory in blocks
  int data_idx; // first block of file-data
};

struct dir_entry {
  bool used; // Is this file-”slot” in use
  char name [MAX_F_NAME + 1]; // DOH!
  int size; // file size
  int head; // first data block of file
  int ref_cnt;
  // how many open file descriptors are there?
  // ref_cnt > 0 -> cannot delete file
};

struct file_descriptor {
  bool used; // fd in use
  int file; // the first block of the file
  // (f) to which fd refers too
  int offset; // position of fd within f
};

struct super_block* fs;
struct file_descriptor fildes[MAX_FILDES]; // 32
short int* FAT; // Will be populated with the FAT data
struct dir_entry* DIR; // Will be populated with
//the directory data

bool mounted = false;

int make_fs(char *disk_name) {
  if (make_disk(disk_name) < 0) {
    return -1;
  }

  if (open_disk(disk_name) < 0) {
    return -1;
  }

  mounted = true;

  fs = calloc(1, sizeof(struct super_block));
  FAT = calloc(4096, sizeof(short int));
  DIR = calloc(64, sizeof(struct dir_entry));

  fs->fat_idx = 1;
  fs->fat_len = 2;
  fs->dir_idx = 3;
  fs->dir_len = 1;
  fs->data_idx = 2048;

  // reserved blocks
  FAT[0] = -3; // super block
  FAT[1] = -3; // FAT
  FAT[2] = -3; // FAT
  FAT[3] = -3; // DIR

  int i;
  for (i = 4; i < 4096; i++) {
    FAT[i] = -2; // free
  }

  if (block_write(0, (char*) fs) < 0) {
    return -1;
  }

  printf("Wrote super block\n");

  for (i = 0; i < 2; i++) {
    if (block_write(fs->fat_idx + i, (char*) (FAT + i*2048)) < 0) {
      return -1;
    }
  }

  printf("Wrote FAT\n");

  if (block_write(fs->dir_idx, (char*) DIR) < 0) {
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

  mounted = true;

  fs = calloc(1, sizeof(struct super_block));
  FAT = calloc(4096, sizeof(int));
  DIR = calloc(64, sizeof(struct dir_entry));

  if (block_read(0, (char*) fs) < 0) {
    return -1;
  }

  printf("Read superblock\n");

  int i;
  for (i = 0; i < 2; i++) {
    if (block_read(fs->fat_idx + i, (char*) (FAT + i*2048)) < 0) {
      return -1;
    }
  }

  printf("Read FAT\n");

  if (block_read(fs->dir_idx, (char*) DIR) < 0) {
    return -1;
  }

  printf("Read DIR\n");

  return 0;
}

int umount_fs(char *disk_name) {

  if (block_write(0, (char*) fs) < 0) {
    return -1;
  }

  printf("Wrote super block to disk\n");

  int i;
  for (i = 0; i < 2; i++) {
    if (block_write(fs->fat_idx + i, (char*) (FAT + i*2048)) < 0) {
      return -1;
    }
  }

  printf("Wrote FAT to disk\n");

  if (block_write(fs->dir_idx, (char*) DIR) < 0) {
    return -1;
  }

  printf("Wrote DIR to disk\n");

  if (close_disk(disk_name) < 0) {
    return -1;
  }

  return 0;
}

int fs_open(char *name) {
  int i, j;
  for (i = 0; i < 64; i++) {
    if (strcmp(DIR[i].name, name) == 0) {
      // file is found
      for (j = 0; j < MAX_FILDES; j++) {
        if (fildes[j].used == false) {
          // free file descriptor
          fildes[j].used = true;
          fildes[j].file = DIR[i].head;
          fildes[j].offset = 0;
          DIR[i].ref_cnt++;
          return j;
        }
      }
      printf("Error: No available file descriptors\n");
      return -1;
    }
  }

  printf("Error: File not found\n");
  return -1;
}

int fs_close(int desc) {
  if ((desc >= MAX_FILDES) || (desc < 0)) {
    return -1;
  }
  if (fildes[desc].used == false) {
    return -1;
  }

  int i;
  for (i = 0; i < 64; i++) {
    if (DIR[i].head == fildes[desc].file) {
      DIR[i].ref_cnt--;
      fildes[desc].used = false;
      fildes[desc].file = -1;
      fildes[desc].offset = 0;
      return 0;
    }
  }

  return -1;
}

int fs_create(char *name) {
  if (strlen(name) > 15) {
    printf("Error: File name too long\n");
    return -1;
  }

  int i;
  int files = 0;
  for (i = 0; i < 64; i++) {
    if (DIR[i].used == true) {
      files++;
      if (strcmp(DIR[i].name, name) == 0) {
        printf("Error: A file already exists with that name\n");
        return -1;
      }
    }
  }

  if (files >= 64) {
    printf("Error: Too many files\n");
    return 0;
  }

  // finds next available block in FAT and marks with eof
  int first_block = -1;
  for (i = 0; i < 4096; i++) {
    if (FAT[i] == -2) { // checks if free
      first_block = i;
      FAT[i] = -1; // eof
      break;
    }
  }

  if (first_block == -1) {
    printf("Error: No more memory available\n");
    return -1;
  }

  // adds to next available DIR entry
  for (i = 0; i < 64; i++) {
    // printf("inside of for loop\n");
    if (DIR[i].used == false) {
      DIR[i].used = true;
      memcpy(DIR[i].name, name, strlen(name));
      DIR[i].size = 1;
      DIR[i].head = first_block;
      DIR[i].ref_cnt = 0;
      printf("Created a file:\n");
      printf("\tName: %s\n", DIR[i].name);
      printf("\tSize: %d blocks\n", DIR[i].size);
      printf("\tHead: Block %d\n", DIR[i].head);
      printf("\tRef Count: %d\n", DIR[i].ref_cnt);
      fs->dir_len++;
      break;
    }
  }

  return 0;
}

int fs_delete(char *name) {
  int i;
  for (i = 0; i < 64; i++) {
    if (strcmp(DIR[i].name, name) == 0) {
      // file is found
      if (DIR[i].ref_cnt > 0) {
        return -1;
      }

      int block = DIR[i].head;
      int next_block;
      while (FAT[block] != -2) {
        next_block = FAT[block];
        FAT[block] = -2;
        if (next_block == -1 || next_block == -3) {
          break;
        }
        block = next_block;
      }

      DIR[i].used = false;
      memcpy(DIR[i].name, " ", 2);
      DIR[i].size = 0;
      DIR[i].head = 0;
      fs->dir_len--;

      return 0;

    }
  }

  printf("Error: Could not find file to delete\n");
  return -1;
}

int fs_read(int desc, void *buf, size_t nbyte) {
  if (fildes[desc].used == false) {
    printf("Error: File not open\n");
    return -1;
  }

  return 0;
}

int fs_write(int fildes, void *buf, size_t nbyte) {
  return 0;
}

int fs_get_filesize(int desc) {
  if (fildes[desc].used == false) {
    printf("Error: File not open\n");
    return -1;
  }

  int i;
  for (i = 0; i < 64; i++) {
    if (DIR[i].head == fildes[desc].file) {
      return DIR[i].size;
    }
  }

  printf("Error: File could not be found\n");
  return -1;
}

int fs_listfiles(char ***files) {
  // char** names = NULL; // = calloc(64, sizeof(char) * 15);
  // *names = malloc(15);
  // names = malloc(64 * sizeof(char*));
  // char* str;
  // str = malloc(15);
  int i = 0;
  // (*files) = malloc(sizeof(char*) * 3); // size of char* x num of files
  // //
  // for (i = 0; i < 3; i++) {
  //   // names[i] = malloc(8);
  //   (*files)[i] = malloc(sizeof(char) * (16));//malloc size of char times file name size plus one
  // }

  for (i = 0; i < 64; i++) {
    if (DIR[i].used == true) {
      printf("\t%s\n", DIR[i].name);
      // strcpy((*files)[j], DIR[i].name);
      // j++;
    }
  }

  // (*files)[j] = NULL;
  // names[j] = NULL;
  // files = &names;
  return 0;
}

int fs_lseek(int fildes, off_t offset) {
  return 0;
}

int fs_truncate(int fildes, off_t length) {
  return 0;
}
