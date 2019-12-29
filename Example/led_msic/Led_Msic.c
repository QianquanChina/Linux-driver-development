#include "linux/init.h"
#include "linux/module.h"
#include "linux/gpio.h"
#include "linux/cdev.h"
#include "linux/fs.h"
#include "linux/uaccess.h"
#include <linux/miscdevice.h>
static long Led_Ioctl(struct file *file, unsigned int cmd, unsigned long buf);

//自定义两个命令
MODULE_LICENSE("GPL");
#define LED_OPEN  0x10001
#define LED_CLOSE 0x10002
struct Led_Resource 
{
    char *name;
    int   gpio;
};

static struct Led_Resource led_info[] =
{
    {
        .name = "Frist_LED",
        .gpio = S5PV210_GPC0(3),
    },
    {
        .name = "Secend_LED",
        .gpio = S5PV210_GPC0(4),
        
    }
};
static struct  file_operations led_fops =       
{
    .unlocked_ioctl = Led_Ioctl,
};

static struct miscdevice misc_dev = 
{
    //动态分配minor设备号
    .minor = MISC_DYNAMIC_MINOR,
    //open所要打开的驱动
    .name  = "myled",
    //文件操作集
    .fops =  &led_fops,
};

/********************************************************
* @功能: Led初始化                                  
* @参数: 无                                            
* @返值: 无
* @说明: 无
*********************************************************/
static int Led_Init(void)
{
    int i, ret;
    //申请GPIO资源，配置为输出 并且输出为0
    for(i=0; i<ARRAY_SIZE(led_info); i++)
    {
        gpio_request(led_info[i].gpio,led_info[i].name);
        gpio_direction_output(led_info[i].gpio, 1);
    }
    //注册混杂设备
    ret = misc_register(&misc_dev);
    if( ret < 0 )
    {
        printk("misc_register fail\n");
    }

    return 0;
}
/********************************************************
* @功能: Led释放                                 
* @参数: 无                                            
* @返值: 无
* @说明: 无
*********************************************************/
static void Led_Exit(void)
{
    int i;
    //输出0释放GPIO资源
    for(i=0; i<ARRAY_SIZE(led_info); i++)
    {
        gpio_set_value(led_info[i].gpio,0);
        gpio_free(led_info[i].gpio);
    }
    misc_deregister(&misc_dev);
}

static long Led_Ioctl(struct file *file, unsigned int cmd, unsigned long buf)
{
    int kindex;
    if( copy_from_user(&kindex, (int *)buf, sizeof(kindex) ) == 0)
    {
        printk("copy from user success\n");
    }

    switch (cmd)
    {
        
    case LED_OPEN:
        gpio_set_value(led_info[kindex - 1].gpio, 1);
        printk("%s:turn open %d led.\n",__func__,kindex-1);
        break;
    case LED_CLOSE:
        gpio_set_value(led_info[kindex - 1].gpio, 0);
        printk("%s:turn close %d led.\n",__func__,kindex-1);
        break;
    default:
        printk("command is invalid\n");
        break;
    }
    return 0;
}

module_init(Led_Init);
module_exit(Led_Exit);