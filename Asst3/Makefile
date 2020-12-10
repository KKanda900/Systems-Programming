CC     = gcc
CFLAGS = -g -Wall -fsanitize=address 

all: Asst3.o
	$(CC) $(CFLAGS) Asst3.o -o KKJserver

clean:
	rm -rf $(TARGET) *.o *.a *.dylib *.dSYM KKJserver
