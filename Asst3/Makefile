CC     = gcc
CFLAGS = -pthread -g -Wall -fsanitize=address 

all: Asst3.o
	$(CC) $(CFLAGS) Asst3.o -lm -o KKJserver

clean:
	rm -rf $(TARGET) *.o *.a *.dylib *.dSYM KKJserver
