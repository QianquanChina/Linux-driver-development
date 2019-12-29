#include "linux/init.h"
#include "linux/module.h"
#include "linux/gpio.h"
#include "linux/cdev.h"
#include "linux/fs.h"
#include "linux/uaccess.h"
static ssize_t Led_Read (struct file *file,char __user * buf, size_t count,loff_t * ppos);

static ssize_t Led_Write(struct file *file,const char __user *buf,size_t count,loff_t *ppos);
MODULE_LICENSE("GPL");

struct Led_Resource 
{
    char *name;
    int   gpio;
};
//write
struct Led_Event
{
    int cmd;
    int index;
};
//read
struct  Led_State
{
    int index;
    int state;
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
    .read  = Led_Read ,
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
        gpio_direction_output(led_info[i].gpio, 1);
    }
    //�����豸��
    alloc_chrdev_region(&dev, 0, 1, "tarena");
    printk("major:  %d, minor:  %d",MAJOR(dev),MINOR(dev));

    //��ʼ���豸���������豸Ӳ�������ӿ�
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
        gpio_set_value(led_info[i].gpio,0);
        gpio_free(led_info[i].gpio);
    }

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
static ssize_t Led_Read (struct file *file,char __user * buf, size_t count,loff_t * ppos)
{
    struct Led_State kledst;
    if( copy_from_user(&kledst, buf, sizeof(kledst)) )
    {
        printk("bad address!\n");
        return -EFAULT;
    }

    kledst.state = gpio_get_value(led_info[kledst.index-1].gpio);
    if( copy_to_user(buf, &kledst, sizeof(kledst)) )
    {
        printk("bad address\n");
        return -EFAULT;
    }
    return count;  
}

module_init(Led_Init);
module_exit(Led_Exit);