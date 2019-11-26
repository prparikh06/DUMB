all:
    gcc -fsanitize=address -std=c99 -o DUMBclient DUMBclient.c
    gcc -fsanitize=address -std=c99 -o DUMBserver DUMBserver.c

client: 
    gcc -fsanitize=address -std=c99 -o DUMBclient DUMBclient.c

serve:
    gcc -fsanitize=address -std=c99 -o DUMBserver DUMBserver.c

clean:
    rm -f *.o $(objects)
	