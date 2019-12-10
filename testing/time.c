#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec"};

char* getTime(){

	time_t t = time(NULL);
	
	struct tm tm = *localtime(&t);
	int military = tm.tm_hour * 100 + tm.tm_min;
	char* month = months[tm.tm_mon];
	int date = tm.tm_mday;
	char day[2];
	sprintf(day, "%d",date);
	char output[20];
	sprintf(output, "%d", military);
	strcat(output," ");
	strcat(output,day);
	strcat(output," ");

	strcat(output,month);
	char* timestamp = malloc(20);
	strcpy(timestamp,output);
	return timestamp;   
   
}

void main(){

	printf("current timestamp: %s\n", getTime());

}
