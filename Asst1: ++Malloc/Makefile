CC = gcc
CFLAGS = 

memgrind: memgrind.o mymalloc.o 
	$(CC) $(CFLAGS) memgrind.o mymalloc.o -o memgrind

memgrind.o: mymalloc.h
mymalloc.o: mymalloc.h

clean:
	rm -f project *.o