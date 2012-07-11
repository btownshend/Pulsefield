#include <stdio.h>
#include <time.h>
#include <sys/time.h>
main()
{
        struct timeval tv;
        gettimeofday(&tv,(void*)0);
	printf("%s",ctime(&tv.tv_sec));
        printf("%ld.%03d\n",tv.tv_sec,tv.tv_usec/1000);
}
