#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
struct key_state
{
    int kstate[4];
    int index[4];
};
struct key_state key;
int main(int argc, char *argv[])
{
    int fd,i;


    fd = open("/dev/mykey", O_RDWR);
    if(fd < 0)
    {
        printf("open led device failed.\n");
        return -1;
    }
    while(1)
    {
        read(fd, &key,sizeof(key) );
        
        for(i=0; i<4; i++)
        {
            printf("第%d个按键的状态是 %d\n", key.index[i], key.kstate[i]);
            sleep(1);
        }

    }
    close(fd);

    return 0;
}
