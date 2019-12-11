# EC440 Intro to Operating Systems Course
### Project 5: File System

written by Laura Joy Erb

I implemented a simple file system on a virtual 32MB disk. I use code from disk.c to interact with the disk, but all functions in fs.c and written by myself.

The file system uses the fs_mount, fs_umount, and fs_make functions to handle the creation of the file system as well as unloading and loading from the virtual disk.

The file system also supports file system functions fs_read, fs_write, fs_lseek, fs_truncate, fs_get_filesize, fs_open, fs_close, fs_create, and fs_delete.

The maximum file size is 16MB. The maximum storage space is 26MB. 
