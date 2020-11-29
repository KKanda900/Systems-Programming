CC     = gcc
CFLAGS = -pthread -g -Wall -fsanitize=address 

all: Asst2.o
	$(CC) $(CFLAGS) Asst2.o -lm -o detector

clean:
	rm -rf $(TARGET) *.o *.a *.dylib *.dSYM detector
