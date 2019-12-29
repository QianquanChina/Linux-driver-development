#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
int main(int argc, char *argv[])
{
    int fd;
    fd = open("/dev/myled", O_RDWR);
    if(fd < 0)
    {
        printf("open led device failed.\n");
        return -1;
    }

    sleep(10);
    close(fd);

    return 0;
}
