#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
struct  Led_Event
{
    int cmd;   //open 1 close 0
    int index; //light number
};
//read
struct  Led_State
{
    int index;
    int state;
};

int main(int argc, char *argv[])
{
    int fd;
    struct Led_Event led_event;
    struct Led_State ledst;//分配用户缓冲区，保存灯的状态

    if(argc > 3)
    {
        printf("Usage: %s <on|off> <1|2>",argv[0]);
        return -1;
    }
    fd = open("/dev/myled", O_RDWR);
    if(fd < 0)
    {
        return -1;
    }
    //指定开关命令
    if( strcmp(argv[1], "Open") == 0 ) 
    {
        led_event.cmd = 1;
    }else
    if( strcmp(argv[1],"Close") == 0 )
    {
        led_event.cmd = 0;
    }
    //指定灯的编号strtoul 将字符串转换成无符号长整型数
    led_event.index = strtoul(argv[2], NULL, 0);
    write( fd, &led_event, sizeof(led_event) );
    printf("123\n");

    //获取编号
    ledst.index = strtoul(argv[2],NULL,0);

    read(fd, &ledst, sizeof(ledst));
    printf("%d  led state is %s\n",ledst.index,ledst.state?"Open":"Close"); 

    close(fd);
    return 0;

}

