#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    int fd;
    if(argc != 2)
    {
        printf("Usage : %s <W|R>\n",argv[0]);
    }
    fd = open("/dev/mybtn", O_RDWR);
    if(fd < 0)
    {
        printf("Open Failed\n");
        return -1;
    }
    if(strcmp(argv[1], "r") == 0)
    {
        read(fd, NULL, 0);
    }else
    if(strcmp(argv[1], "w") == 0)
    {
        write(fd, NULL, 0);
    }
    close(fd);
    return 0;
}