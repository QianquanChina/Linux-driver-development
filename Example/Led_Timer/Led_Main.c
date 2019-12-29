#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    int fd;
    fd = open("/dev/myled", O_RDWR);
    if (fd < 0)
    {
        printf("failed open\n");
        return -1;
    }
}