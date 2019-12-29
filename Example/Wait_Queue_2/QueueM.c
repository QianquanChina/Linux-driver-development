#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//声明按键信息的数据结构
struct btn_event{
    int code;  //键值
    int state; //状态
};
int main(int argc, char *argv[])
{
    int fd;
    struct btn_event btn;  //分配用户缓冲区
    fd = open("/dev/mybtn", O_RDWR);
    if(fd < 0)
    {
        printf("Open Failed\n");
        return -1;
    }
    while(1)
    {
        read(fd,&btn,sizeof(btn));
        printf("keycode is %d, state is %s\n",btn.code,btn.state?"up":"down");
    }
    close(fd);
    return 0;
}