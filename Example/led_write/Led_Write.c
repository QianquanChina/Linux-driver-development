#include "linux/init.h"
#include "linux/module.h"
#include "linux/gpio.h"
#include "linux/cdev.h"
#include "linux/fs.h"
#include "linux/uaccess.h"
#include "linux/device.h"

static ssize_t Led_Write(struct file *file,const char __user *buf,size_t count,loff_t *ppos);
MODULE_LICENSE("GPL");

struct Led_Resource 
{
    char *name;
    int   gpio;
};
struct Led_Event
{
    int cmd;
    int index;
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
    .write = Led_Write,
};

static struct  cdev led_cdev;
static dev_t dev;
static struct class *cls;

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
    //创建树干
    cls = class_create(THIS_MODULE, "QianQuan");
    device_create(cls, NULL, MKDEV(MAJOR(dev), 0), NULL, "myled");
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
    device_destroy(cls, MKDEV(MAJOR(dev), 0) );
    class_destroy(cls);

}
static ssize_t Led_Write(struct file *file,const char __user *buf,size_t count,loff_t *ppos)
{
    struct Led_Event kled_event;

    if( copy_from_user(&kled_event, buf, sizeof(kled_event)) )
    {
        printk("bad address\n");
        return -EFAULT;
    }
    gpio_set_value(led_info[ kled_event.index -1 ].gpio, kled_event.cmd);
    printk("%s: %s %d led\n",__func__,kled_event.cmd?"Open":"Close",kled_event.index);
    return count;
}

module_init(Led_Init);
module_exit(Led_Exit);