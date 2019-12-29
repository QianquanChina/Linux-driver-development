#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//����������Ϣ�����ݽṹ
struct btn_event{
    int code;  //��ֵ
    int state; //״̬
};
int main(int argc, char *argv[])
{
    int fd;
    struct btn_event btn;  //�����û�������
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