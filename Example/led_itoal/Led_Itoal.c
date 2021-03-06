#include "linux/init.h"
#include "linux/module.h"
#include "linux/gpio.h"
#include "linux/cdev.h"
#include "linux/fs.h"
#include "linux/uaccess.h"
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

static struct  cdev led_cdev;
static dev_t dev;

static int Led_Init(void)
{
    int i;
    //申请GPIO资源，配置为输出 并且输出为0
    for(i=0; i<ARRAY_SIZE(led_info); i++)
    {
        gpio_request(led_info[i].gpio,led_info[i].name);
        gpio_direction_output(led_info[i].gpio, 1);
    }
    //申请设备号
    alloc_chrdev_region(&dev, 0, 1, "tarena");
    printk("major:  %d, minor:  %d",MAJOR(dev),MINOR(dev));

    //初始化设备对象，添加设备硬件操作接口
    cdev_init(&led_cdev, &led_fops);

    //向内核中注册字符设备对象
    cdev_add(&led_cdev,dev,1);
    return 0;
}
static void Led_Exit(void)
{
    int i;
    
    //从内核中卸载字符设备对象
    cdev_del(&led_cdev);

    //释放设备号
    unregister_chrdev_region(dev, 1);
    //输出0释放GPIO资源
    for(i=0; i<ARRAY_SIZE(led_info); i++)
    {
        gpio_set_value(led_info[i].gpio,0);
        gpio_free(led_info[i].gpio);
    }

}

static long Led_Ioctl(struct file *file, unsigned int cmd, unsigned long buf)
{
    int i;
    switch (cmd)
    {
    case LED_OPEN:
        for ( i = 0; i < ARRAY_SIZE(led_info); i++)
        {
            gpio_set_value(led_info[i].gpio, 1);
            printk("%s:turn on %d led.\n",__func__, i+1);
        }        
        break;
    case LED_CLOSE:
        for ( i = 0; i < ARRAY_SIZE(led_info); i++)
        {
            gpio_set_value(led_info[i].gpio, 0);
            printk("%s:turn on %d led.\n",__func__, i+1);
        }
        break;
    default:
        printk("command is invalid\n");
        break;
    }
}

module_init(Led_Init);
module_exit(Led_Exit);