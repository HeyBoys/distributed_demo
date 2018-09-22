#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<stdio.h>
#include<sys/types.h>
int main()
{
	int fd;
	fd = open("cc/hello",O_WRONLY|O_CREAT,0644);
	if(fd<0)
	{
		printf("open error\n");
		return 0;
	}
	char buf[1000] = "askldasldaslkdjlasjdlajdla932dfsn,mnkj3nkr3jn,asmn,as190u0ueq08w9udsadjaos\0";
//	for(int i = 0;i<2000;i++)		
	printf("%s\n",buf);
		write(fd,buf,strlen(buf));
}
