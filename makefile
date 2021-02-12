OUTPUTS = client server
CC = gcc

all: $(OUTPUTS)

clean:
	rm -f $(OUTPUTS) *.o

.PHONY: all clean

%.o: %.c
	$(CC) -c -o $@ $<

client: client.o
	$(CC) -o $@ $^

server: server.o
	$(CC) -o $@ $^
