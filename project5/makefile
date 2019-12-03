CC=gcc -Werror -Wall -g -m32

all: fs main.c
	$(CC) -lpthread -o main main.c fs.o

fs: fs.c
	$(CC) -lpthread -c -o fs.o fs.c

clean:
	rm main fs.o
