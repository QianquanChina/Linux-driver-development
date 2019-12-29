#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

int main(int argc, char *argv[])
{
    int fd;
    void *gpiobase;
    unsigned long *gpiocon;
    unsigned long *gpiodata;
    if (argc != 2)
    {
        printf("Usage: %s <on|off>\n", argv[0]);
        /* code */
    }
    fd = open("/dev/myled",O_RDWR);
    if (fd < 0)
    {
        printf("Open failed");
        /* code */
    }
    gpiobase = mmap(NULL, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    printf("gpiobase = %#x\n",gpiobase);
    gpiocon  = (unsigned long *)(gpiobase + 0x60);
    gpiodata = (unsigned long *)(gpiobase + 0x64);
    printf("gpiocon = %#x, gpiodata = %#x\n",gpiocon, gpiodata);
    //配置GPIO为输出口，输出0
    *gpiocon &= ~((0xF << 12) | (0xF << 16));
    *gpiocon |= ((1 << 12) | (1 << 16));
    *gpiodata &= ~((1 << 3)|(1<<4));
    if(!strcmp(argv[1],"on"))
    {
        *gpiodata |= ((1 << 3)|(1<<4));
    }else 
    if(!strcmp(argv[1],"off"))
    {
        *gpiodata &= ~((1 << 3)|(1<<4));
    }   
    munmap(gpiobase,0x1000);
    close(fd);
}