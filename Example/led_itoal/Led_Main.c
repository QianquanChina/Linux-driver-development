#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//自定义两个命令
#define LED_OPEN  0x10001
#define LED_CLOSE 0x10002

int main(int argc, char *argv[])
{
    int fd;
    if(argc != 2)
    {
        printf("Usage: %s <Open|Close><1|2>",argv[0]);
        return -1;
    }
    fd = open("/dev/myled",O_RDWR);
    if(strcmp(argv[1], "Open"))
    {
        ioctl(fd, LED_OPEN);
        printf("Open\n");
    }else
    if(strcmp(argv[1], "Close"))
    {
        ioctl(fd, LED_CLOSE);
        printf("Close\n");
    }
    close(fd);
    
    return 0;

}