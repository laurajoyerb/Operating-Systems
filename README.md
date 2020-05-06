# Operating-Systems
EC440: Intro to Operating Systems course
taught by Professor Manuel Egele

This repo is a collection of all projects completed for the Operating Systems course at Boston University. 

### [Project 1](./project1) - Simple Shell
The goal of this project was to implement a basic shell which is able to execute commands, redirect the
standard input/output (stdin/stdout) of commands to files, pipe the output of commands to other
commands, and carry out commands in the background.

### [Project 2](./project2) - User Mode Thread Library
The main deliverable for this project was a basic thread system for Linux. In the lectures, we
learned that threads are independent units of execution that run (virtually) in parallel in the
address space of a single process. As a result, they share the same heap memory, open files
(file descriptors), process identifier, etc. Each thread has its own context, which consists of a set
of CPU registers and a stack. The thread subsystem provides a set of library functions that
applications may use to create, start and terminate threads, and manipulate them in various
ways.

### [Project 3](./project3) - Thread Synchronization
In Project 2, we implemented a basic threading library that enables a developer to create and
perform computations in multiple threads. In this Project, we extended the library to
facilitate communication between threads. This included adding support for a locking mechanism, the pthread_join POSIX thread function, and semaphores.

### [Project 4](./project4) - Thread Local Storage
The goal of this project was to implement a library that provides protected memory regions for
threads, which they can safely use as local storage. All threads share the
same memory address space. While it can be beneficial since it allows threads to easily share
information, it can also be a problem when one thread accidentally modifies values that
another thread has stored in a variable. To protect data from being overwritten by other
threads, it would be convenient for each thread to possess a protected storage area that only
this thread can read from and write to. We call this protected storage area thread local
storage. The task was to implement support for thread local storage on top of Linux' pthread implementation.

### [Project 5](./project5) - File System
The goal of this project was to implement a simple file system on top of a virtual disk. To this end,
we implemented a library that offers a set of basic file system calls (such as open, read,
write, ...) to applications. The file data and file system meta-information will be stored on a
virtual disk. This virtual disk is actually a single file that is stored on the "real" file system
provided by the Linux operating system. That is, we basically implemented a file
system on top of the Linux file system.
To create and access the virtual disk, the professor provided a few definitions and helper functions
that can be found in disk.h and disk.c. As you can see by looking at the provided header and source
files, the virtual disk has 8,192 blocks, and each block holds 4KB. You can create an empty
disk, open and close a disk, and read and write entire blocks (by providing a block number in
the range between 0 and 8,191 inclusive).
