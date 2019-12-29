#include "linux/init.h"
#include "linux/module.h"
#include "linux/gpio.h"
#include "linux/cdev.h"
#include "linux/fs.h"
#include "linux/uaccess.h"
#include <linux/miscdevice.h>
static long Led_Ioctl(struct file *file, unsigned int cmd, unsigned long buf);

//�Զ�����������
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
    //��̬����minor�豸��
    .minor = MISC_DYNAMIC_MINOR,
    //open��Ҫ�򿪵�����
    .name  = "myled",
    //�ļ�������
    .fops =  &led_fops,
};

/********************************************************
* @����: Led��ʼ��                                  
* @����: ��                                            
* @��ֵ: ��
* @˵��: ��
*********************************************************/
static int Led_Init(void)
{
    int i, ret;
    //����GPIO��Դ������Ϊ��� �������Ϊ0
    for(i=0; i<ARRAY_SIZE(led_info); i++)
    {
        gpio_request(led_info[i].gpio,led_info[i].name);
        gpio_direction_output(led_info[i].gpio, 1);
    }
    //ע������豸
    ret = misc_register(&misc_dev);
    if( ret < 0 )
    {
        printk("misc_register fail\n");
    }

    return 0;
}
/********************************************************
* @����: Led�ͷ�                                 
* @����: ��                                            
* @��ֵ: ��
* @˵��: ��
*********************************************************/
static void Led_Exit(void)
{
    int i;
    //���0�ͷ�GPIO��Դ
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