#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<stdio.h>
#include<sys/types.h>
int main()
{
	int fd;
	fd = open("cc/hello",O_RDONLY);
	if(fd<0)
	{
		printf("open error\n");
		return 0;
	}
	char buf[10000];	
	int aa = read(fd,buf,50);
//write(1,buf,50);
	printf("length:%d\n",aa);
}	
	
