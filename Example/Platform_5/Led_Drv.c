#include <linux/init.h>
#include <linux/module.h>
#include "linux/fs.h"
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#define LED_ON  0x100001
#define LED_OFF 0x100002
/*----------��������---------------*/

static int  Led_DevInit(void);
static void Led_DevExit(void);
static long Led_Ioctl(struct file *file, unsigned int cmd, unsigned long buf);
static int  Led_Remove(struct platform_device *pdev);
static int  Led_Probe(struct platform_device *pdev);
/*----------��������---------------*/
struct Led_Resource
{
    unsigned long gpio_phys_base;//�Ĵ�����ʼ��ַ
    unsigned long size;//�Ĵ����ĵ�ַ�ռ��С
    unsigned long gpio;//GPIO���

};
static struct platform_driver led_drv = 
{
    .driver = 
            {
                .name = "led_drv"
            },
    .probe  = Led_Probe, //ƥ��ɹ����ں˵���
    .remove = Led_Remove,//ɾ���������Ӳ���ڵ㣬�ں˵���
};
//��������LEDӲ�������ݽṹ
static void *gpiobase;
static unsigned long *gpiocon;
static unsigned long *gpiodata;
static int  pin;

static struct file_operations led_fops = 
{
    .unlocked_ioctl = Led_Ioctl
};
//��������豸����
static struct miscdevice led_misc = 
{
    .minor = MISC_DYNAMIC_MINOR,
    .name  = "myled",
    .fops  = &led_fops,  
};

static int Led_DevInit(void)
{
    //1.ע������ڵ����drv����
    //������ƥ�䡢����probe�ȶ����ں����
    platform_driver_register(&led_drv);   
    return 0;
}
static long Led_Ioctl(struct file *file, unsigned int cmd, unsigned long buf)
{
    int kindex;
    copy_from_user( &kindex, (int *)buf, sizeof(kindex) );
    //����ָ��
    switch (cmd)
    {
        case LED_ON:
            if(kindex == 1)
            {
                *gpiodata |= (1 << pin);
            }
            break;
        
        case LED_OFF:
            if(kindex == 1)
            {
                *gpiodata &= ~(1 << pin);
            }
            break;

        default:
            break;
    }
    return 0;
}
//ƥ��ɹ� �ں��Զ�����
//pdevָ��ƥ��ɹ��Ľڵ㣬For example ��pdev = led_dev.c�е�&led_dev
static int Led_Probe(struct platform_device *pdev)
{
    //��ȡ�Զ����Ӳ����Ϣ ����� pdata=led_dev.c��&led_info
    struct Led_Resource *pdata = pdev->dev.platform_data;
    //pin = 3;
    pin = pdata->gpio;

    //���ִ���
    //���Ĵ����������ַӳ�䵽�ں������ַ
    gpiobase = ioremap(pdata->gpio_phys_base, pdata->size);
    gpiocon  = (unsigned long *)(gpiobase + 0x00);
    gpiodata = (unsigned long *)(gpiobase + 0x04);

    //gpio ����ģʽ
    *gpiocon  &= ~(0x0F << (4*pin)); 
    *gpiocon  |= (1 << (4*pin));
    *gpiodata &= ~(1 << pin); 
    printk("%s\n",__func__);

    /*����ط�ע������豸������???*/
    misc_register(&led_misc);
    return 0;
}
//ɾ���������Ӳ�����ں��Զ�����
//pdevָ��ƥ��ɹ���Ӳ���ڵ㣬����pdev=led_dev.c�е�&led_dev
static int Led_Remove(struct platform_device *pdev)
{
    //1.ж�ػ����豸
    misc_deregister(&led_misc);
    //2.���0
    *gpiodata &= ~(1 << pin);
    //3.�����ַӳ��
    iounmap(gpiobase);
    return 0;
}
static void Led_DevExit(void)
{
    //��drv����ɾ������ڵ�
    platform_driver_unregister(&led_drv);
}
module_init(Led_DevInit);
module_exit(Led_DevExit);
MODULE_LICENSE("GPL");