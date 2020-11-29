CC     = gcc
CFLAGS = -pthread -g -Wall -fsanitize=address 

Asst2: Asst2.o
	$(CC) $(CFLAGS) Asst2.o -lm -o detector

clean:
	rm -rf $(TARGET) *.o *.a *.dylib *.dSYM detector
