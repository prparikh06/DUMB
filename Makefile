all:	client serve queue.o

client: queue.o
	gcc -w -fsanitize=address -std=c99 -o DUMBclient DUMBclient.c queue.o

serve: queue.o
	gcc -w -fsanitize=address -std=c99 -o DUMBserver DUMBserver.c queue.o

queue.o: queue.c
	gcc -c -std=c99 queue.c

clean:
	rm -f DUMBclient DUMBserver; rm *.o
	
