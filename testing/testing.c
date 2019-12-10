#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(){

	printf("please input a string: \n");
	int c, i = 0;
	char *str = malloc(i+1);
	str[i++] = '\0';
	char* tmp;
	while((c = getchar()) != EOF && c!= '\n' && (tmp = realloc(str,i+1)) != NULL){
		str = tmp;
		str[i-1] = c;
		str[i++] = '\0';
		//printf("current str = %s\n", str);
	}

	printf("your string is: \n%s\n", str);
	free(str);

}
