#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//自定义两个命令
#define LED_OPEN  0x100001
#define LED_CLOSE 0x100002

int main(int argc, char *argv[])
{
    int fd;
    int index;
    if(argc != 3)
    {
        printf("Usage: %s <Open|Close><1|2>",argv[0]);
        return -1;
    }
    fd = open("/dev/myled",O_RDWR);

    //获取灯的编号
    index=strtoul(argv[2],NULL,0);
    if(strcmp(argv[1], "Open"))
    {
        ioctl(fd, LED_OPEN, &index);
        printf("Open\n");
    }else
    if(strcmp(argv[1], "Close"))
    {
        ioctl(fd, LED_CLOSE, &index);
        printf("Close\n");
    }
    close(fd);
    
    return 0;

}