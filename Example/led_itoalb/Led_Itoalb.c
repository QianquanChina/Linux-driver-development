#include "linux/init.h"
#include "linux/module.h"
#include "linux/gpio.h"
#include "linux/cdev.h"
#include "linux/fs.h"
#include "linux/uaccess.h"
static long Led_Ioctl(struct file *file, unsigned int cmd, unsigned long buf);

//�Զ�����������
MODULE_LICENSE("GPL");
#define LED_OPEN  0x100001
#define LED_CLOSE 0x100002
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
    //����GPIO��Դ������Ϊ��� �������Ϊ0
    for(i=0; i<ARRAY_SIZE(led_info); i++)
    {
        gpio_request(led_info[i].gpio,led_info[i].name);
        gpio_direction_output(led_info[i].gpio, 1);
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
        gpio_set_value(led_info[i].gpio,0);
        gpio_free(led_info[i].gpio);
    }

}

static long Led_Ioctl(struct file *file, unsigned int cmd, unsigned long buf)
{
    int kindex;
    copy_from_user(&kindex, (int *)buf, sizeof(kindex) );

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