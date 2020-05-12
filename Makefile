CC=g++
CFLAGS=-I.

client: client.cpp helpers.cpp requests.cpp buffer.cpp
	$(CC) -o client client.cpp helpers.cpp requests.cpp buffer.cpp -Wall

run: client
	./client

clean:
	rm -f *.o client
