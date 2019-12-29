#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//自定义两个命令
#define LED_OPEN  0x10001
#define LED_CLOSE 0x10002

int main(int argc, char *argv[])
{
    int fd0,fd1;
    int index;
    fd0 = open("/dev/myled0",O_RDWR);
    if(fd0 < 0)
    {
        printf("open led device failed.\n");
            return -1;
    }       
    fd1 = open("/dev/myled1",O_RDWR);
    if(fd1 < 0)
    {
        printf("open led device failed.\n");
            return -1;
    }   
    while (1)
    {
        ioctl(fd0,LED_OPEN);
        sleep(3);
        ioctl(fd1,LED_OPEN);
        sleep(3);
        ioctl(fd0,LED_CLOSE);
        sleep(3);
        ioctl(fd1,LED_CLOSE);
        sleep(3);       
    }
    
    close(fd1);
    close(fd0);
    return 0;

}