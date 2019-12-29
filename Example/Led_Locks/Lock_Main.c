#include "stdlib.h"
#include "stdio.h"
#include "sys/types.h"
#include "fcntl.h"
int main(int argc, char *argv[])
{
    int fd;
    fd = open("/dev/myled",O_RDWR);
    if (fd < 0)
    {
        printf("open device failded \n");
        return -1;
    }
    sleep(100000);
    close(fd);
    return 0;
}