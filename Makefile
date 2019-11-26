all:	client serve

client: 
	gcc -fsanitize=address -std=c99 -o DUMBclient DUMBclient.c

serve:
	gcc -fsanitize=address -std=c99 -o DUMBserver DUMBserver.c

clean:
	rm -f DUMBclient DUMBserver
	
