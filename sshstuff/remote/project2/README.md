# Project 2 
EC440: Intro to Operating Systems course
Laura Joy Erb
Fall 2019
Professor Manuel Egele

## User Mode Thread Library

This project implements a user-mode thread library that allows for parallel execution of threads within a process.

## Challenges
I struggled initially with correctly allocating memory for the stack of each thread. Ensuring that the arg, start_routine address, PC, pthread_exit address, and the stack pointer were all correctly assigned in pthread_create was the most challenging part of the assignment for me. 

## Functionality
The library correctly implements pthread_create, pthread_exit, and pthread_self. It uses schedule() to implement a round robin scheduling routine that preemptively switches thread contexts every 50ms. It can handle up to 128 threads of parallel execution. Any further calls to pthread_create will not create a new thread until one of the old threads exits and opens a space for the new one. 
