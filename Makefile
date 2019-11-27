all:	client serve

client: 
	gcc -w -fsanitize=address -std=c99 -o DUMBclient DUMBclient.c

serve:
	gcc -w -fsanitize=address -std=c99 -o DUMBserver DUMBserver.c

clean:
	rm -f DUMBclient DUMBserver
	
