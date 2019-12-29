#include "linux/gpio.h"
#include "linux/init.h"
#include "linux/module.h"
#include "linux/cdev.h"
#include "linux/fs.h"
#include <linux/device.h>

MODULE_LICENSE("GPL");

static int Led_Open(struct inode *inde, struct file *file);
static int Led_Close(struct inode *inde, struct file *file);

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
    .open    = Led_Open,
    .release = Led_Close,
};

static struct  cdev led_cdev;
static dev_t dev;

static int Led_Init(void)
{
    int i;
    //����GPIO��Դ������Ϊ��� �������Ϊ0
    for(i=0; i<ARRAY_SIZE(led_info); i++)
    {
        gpio_request(led_info[i].gpio,led_info[i].name);
        gpio_direction_output(led_info[i].gpio, 0);
    }
    //�����豸��
    alloc_chrdev_region(&dev, 0, 1, "tarena");
    printk("major:  %d, minor:  %d",MAJOR(dev),MINOR(dev));

    //��ʼ���豸��������豸Ӳ�������ӿ�
    cdev_init(&led_cdev, &led_fops);

    //���ں���ע���ַ��豸����
    cdev_add(&led_cdev,dev,1);
    return 0;
}
static void Led_Exit(void)
{
    int i;
    
    //���ں���ж���ַ��豸����
    cdev_del(&led_cdev);

    //�ͷ��豸��
    unregister_chrdev_region(dev, 1);
    //���0�ͷ�GPIO��Դ
    for(i=0; i<ARRAY_SIZE(led_info); i++)
    {
        gpio_set_value(led_info[i].gpio,1);
        gpio_free(led_info[i].gpio);
    }

}
static int Led_Open(struct inode *inde, struct file *file)
{
    int i;
    for(i=0; i<ARRAY_SIZE(led_info); i++)
    {
        gpio_set_value(led_info[i].gpio, 1);
        printk("%s:�򿪵�%d����\n",__func__,i+1);
    }
    return 0;
}
static int Led_Close(struct inode *inde, struct file *file)
{
    int i;
    for(i=0; i<ARRAY_SIZE(led_info); i++)
    {
        gpio_set_value(led_info[i].gpio, 0);
        printk("%s:�رյ�%d����\n",__func__,i+1);
        
    }
    return 0;

}

module_init(Led_Init);
module_exit(Led_Exit);