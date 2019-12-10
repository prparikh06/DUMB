#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>


void main(int argc, char* argv[]){
	char* hostname = argv[1];

  	struct hostent *host = gethostbyname(hostname);
	if (host == NULL) printf("error!\n");
	char* ip = inet_ntoa(*((struct in_addr*) host->h_addr_list[0]));
	printf("ip = %s\n", ip);

}


