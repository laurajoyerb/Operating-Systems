#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "disk.h"
#include "fs.h"

#define MAX_F_NAME 15
#define MAX_FILDES 32
#define SUPER_BLOCK_NUM 0
#define MAX_BLOCK_SIZE 4096
#define MAX_FILE_SIZE (16 * 1024 * 1024)

struct super_block {
  int fat_idx; // first block of FAT
  int fat_len; // length of FAT in blocks
  int dir_idx; // first block of directory
  int dir_len; // length of directory in files
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
  FAT = calloc(8192, sizeof(short int));
  DIR = calloc(64, sizeof(struct dir_entry));

  fs->fat_idx = 1;
  fs->fat_len = 4;
  fs->dir_idx = 5;
  fs->dir_len = 0;
  fs->data_idx = 6;

  // reserved blocks
  FAT[0] = -3; // super block
  FAT[1] = -3; // FAT
  FAT[2] = -3; // FAT
  FAT[3] = -3; // DIR

  int i;
  for (i = 4; i < 8192; i++) {
    FAT[i] = -2; // free
  }

  if (block_write(0, (char*) fs) < 0) {
    return -1;
  }

  for (i = 0; i < fs->fat_len; i++) {
    if (block_write(fs->fat_idx + i, (char*) (FAT + i*2048)) < 0) {
      return -1;
    }
  }

  if (block_write(fs->dir_idx, (char*) DIR) < 0) {
    return -1;
  }

  if (close_disk(disk_name) < 0) {
    return -1;
  }

  printf("File system made\n");

  return 0;
}

int mount_fs(char *disk_name) {

  if (open_disk(disk_name) < 0) {
    return -1;
  }

  mounted = true;

  fs = calloc(1, sizeof(struct super_block));
  FAT = calloc(8192, sizeof(int));
  DIR = calloc(64, sizeof(struct dir_entry));

  if (block_read(0, (char*) fs) < 0) {
    return -1;
  }

  int i;
  for (i = 0; i < fs->fat_len; i++) {
    if (block_read(fs->fat_idx + i, (char*) (FAT + i*2048)) < 0) {
      return -1;
    }
  }

  if (block_read(fs->dir_idx, (char*) DIR) < 0) {
    return -1;
  }

  for (i = 0; i < MAX_FILDES; i++) {
    fildes[i].used = false;
    fildes[i].file = -1;
    fildes[i].offset = 0;
  }

  for (i = 0; i < 64; i++) {
    DIR[i].ref_cnt = 0;
  }

  printf("File system mounted\n");

  return 0;
}

int umount_fs(char *disk_name) {

  if (block_write(0, (char*) fs) < 0) {
    return -1;
  }

  int i;
  for (i = 0; i < fs->fat_len; i++) {
    if (block_write(fs->fat_idx + i, (char*) (FAT + i*2048)) < 0) {
      return -1;
    }
  }

  if (block_write(fs->dir_idx, (char*) DIR) < 0) {
    return -1;
  }

  if (close_disk(disk_name) < 0) {
    return -1;
  }

  printf("File system written to disk\n");

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
  if (strlen(name) > 16) {
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
  for (i = fs->data_idx; i < 8192; i++) {
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
      DIR[i].size = 0;
      DIR[i].head = first_block;
      DIR[i].ref_cnt = 0;
      printf("Created a file:\n");
      printf("\tName: %s\n", DIR[i].name);
      printf("\tSize: %d bytes\n", DIR[i].size);
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
  int file_off = fildes[desc].offset;
  int file_block = fildes[desc].file; // starts at head

  if (file_off == fs_get_filesize(desc)) {
    return 0;
  }

  while (file_off >= MAX_BLOCK_SIZE) { // moves offset up blocks if need be
    file_off -= MAX_BLOCK_SIZE; // gets offset for that block
    file_block = FAT[file_block]; // grabs next block
    if (file_block == -1) {
      // has reached end of file, eof is found before offset is less than block size
      return 0;
    }
  }

  char block_contents[MAX_BLOCK_SIZE];
  memset(block_contents, -1, MAX_BLOCK_SIZE);
  if (block_read(file_block, block_contents) < 0) {
    printf("Error: Could not read from block %d\n", file_block);
    return -1;
  }

  int bytes_read = 0;
  char read_contents[nbyte];
  memset(read_contents, 0, nbyte);

  int i;
  for (i = file_off; i < MAX_BLOCK_SIZE; i++) {
    // reaches end of file
    if (block_contents[i] == -1) {
      memcpy(buf, read_contents, nbyte);
      return bytes_read;
    }

    read_contents[bytes_read] = block_contents[i];
    bytes_read++;
    fildes[desc].offset++;

    if (nbyte == bytes_read) {
      memcpy(buf, read_contents, nbyte);
      return bytes_read;
    }
  }
  while(nbyte > bytes_read) {
    // more than one block to read
    file_block = FAT[file_block];
    if (file_block == -1) {
      memcpy(buf, read_contents, nbyte);
      return bytes_read;
    }

    memset(block_contents, -1, MAX_BLOCK_SIZE);
    if (block_read(file_block, block_contents) < 0) {
      printf("Error: Could not read from block %d\n", file_block);
      return -1;
    }

    for (i = 0; i < MAX_BLOCK_SIZE; i++) {
      if (block_contents[i] == -1) {
        memcpy(buf, read_contents, nbyte);
        return bytes_read;
      }

      read_contents[bytes_read] = block_contents[i];
      bytes_read++;
      fildes[desc].offset++;

      if (nbyte == bytes_read) {
        memcpy(buf, read_contents, nbyte);
        return bytes_read;
      }
    }
  }

  memcpy(buf, read_contents, nbyte);
  return bytes_read;
}

int fs_write(int desc, void *buf, size_t nbyte) {
  if (fildes[desc].used == false) {
    printf("Error: File not open\n");
    return -1;
  }

  int i, desc_dir;
  for (i = 0; i < 64; i++) {
    if (DIR[i].head == fildes[desc].file) {
      desc_dir = i;
      break;
    }
  }

  if (nbyte + DIR[desc_dir].size > MAX_FILE_SIZE) {
    nbyte = MAX_FILE_SIZE - DIR[desc_dir].size;
  }

  int file_off = fildes[desc].offset;
  int file_block = fildes[desc].file; // starts at head

  while (file_off > MAX_BLOCK_SIZE) { // moves offset up blocks if need be
    file_off -= MAX_BLOCK_SIZE; // gets offset for that block
    file_block = FAT[file_block]; // grabs next block
  }

  if (FAT[file_block] == -1) {
    // empty file, must allocate another block on FAT
    // finds next available block in FAT and marks with eof
    int first_block = -1;
    for (i = fs->data_idx; i < 8192; i++) {
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

    FAT[file_block] = first_block;
    FAT[first_block] = -1; // new eof
    if (nbyte < MAX_BLOCK_SIZE) {
      DIR[desc_dir].size += nbyte;
    } else {
      DIR[desc_dir].size += MAX_BLOCK_SIZE; // increments size of file in directory entry
    }
  }

  char block_contents[MAX_BLOCK_SIZE];
  if (block_read(file_block, block_contents) < 0) {
    printf("Error: Could not read from block %d\n", file_block);
    return -1;
  }

  int bytes_written = 0;
  char write_contents[nbyte];
  memcpy(write_contents, buf, nbyte);

  for (i = file_off; i < MAX_BLOCK_SIZE; i++) {
    block_contents[i] = write_contents[bytes_written];
    bytes_written++;
    fildes[desc].offset++;

    if (nbyte == bytes_written) {
      block_write(file_block, block_contents);
      return bytes_written;
    }
  }

  bool new_block = false;
  while (nbyte > bytes_written) {
    if (new_block) {
      DIR[desc_dir].size += MAX_BLOCK_SIZE; // adds last block of size to file
      new_block = false;
    }
    file_block = FAT[file_block]; // grabs next block

    if (FAT[file_block] == -1) {
      // empty file, must allocate another block on FAT
      // finds next available block in FAT and marks with eof
      int first_block = -1;
      for (i = fs->data_idx; i < 8192; i++) {
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

      FAT[file_block] = first_block;
      FAT[first_block] = -1; // new eof
      new_block = true;
    }

    if (block_read(file_block, block_contents) < 0) {
      printf("Error: Could not read from block %d\n", file_block);
      return -1;
    }

    for (i = 0; i < MAX_BLOCK_SIZE; i++) {
      block_contents[i] = write_contents[bytes_written];
      bytes_written++;
      fildes[desc].offset++;

      if (nbyte == bytes_written) {
        block_write(file_block, block_contents);
        if (new_block) {
          DIR[desc_dir].size += (i + 1);
        }
        return bytes_written;
      }
    }

  }

  block_write(file_block, block_contents);
  return bytes_written;
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
  int i, j = 0;

  char* files_list[64] = {"0"};

  for (i = 0; i < 64; i++) {
    if (DIR[i].used == true) {
      files_list[j] = DIR[i].name;
      j++;
    }
  }
  files_list[j] = NULL;

  *files = files_list;
  return 0;
}

int fs_lseek(int desc, off_t offset) {
  if (offset > fs_get_filesize(desc) || offset < 0) {
    printf("Error: Offset is out of bounds for block\n");
    return -1;
  }

  if (fildes[desc].used == false) {
    printf("Error: Not a valid file descriptor\n");
    return -1;
  }

  fildes[desc].offset = offset;
  printf("New offset for file %d is %d\n", desc, fildes[desc].offset);
  return 0;
}

int fs_truncate(int desc, off_t length) {
  if (fildes[desc].used == false) {
    return -1;
  }

  int i, desc_dir = -1;
  for (i = 0; i < 64; i++) {
    if (DIR[i].head == fildes[desc].file) {
      desc_dir = i;
      break;
    }
  }

  if (desc_dir < 0) {
    printf("Error: File could not be found in directory\n");
    return -1;
  }

  if (DIR[desc_dir].size < length) {
    return -1;
  }

  if (DIR[desc_dir].size == length) {
    return 0;
  }

  DIR[desc_dir].size = length;
  if (fildes[desc].offset > length) {
    fildes[desc].offset = length;
  }

  int trunc_block = length / MAX_BLOCK_SIZE;
  trunc_block++;

  int curr_block = DIR[desc_dir].head;

  for (i = 0; i < trunc_block; i++) {
    curr_block = FAT[curr_block];
  }

  int erase_block = FAT[curr_block];
  FAT[curr_block] = -1; // eof

  while (erase_block > 0) {
    erase_block = FAT[erase_block];
    FAT[erase_block] = -2; // frees block
  }
  return 0;
}
