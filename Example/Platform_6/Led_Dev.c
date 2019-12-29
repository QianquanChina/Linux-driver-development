#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
/*-------------��������-------------*/
static int Led_DevInit(void);
static void Led_DevExit(void);
static void Led_Release(struct device *dev);
/*-------------��������-------------*/

//��������LEDӲ����Ϣ�����ݽṹ
struct Led_Resource
{
    /* data */
    unsigned long gpio_phys_base;   //�Ĵ�����ʼ��ַ
    unsigned long size;             //�Ĵ����ĵ�ַ��С�ռ�
    unsigned long gpio;             //GPIO���
};

//�����ʼ������LED��Ӳ����Ϣ����-----Apple
static struct Led_Resource led_info = 
{
    .gpio_phys_base = 0xE0200060,
    .size           = 8,
    .gpio           = 3,
};

//�����ʼ��LED��Ӳ���ڵ����----Box
static struct platform_device led_dev = 
{
    .name = "led_drv",
    .id   = -1,
    .dev  = {
                .platform_data = &led_info, //װ��Ӳ����Ϣ Fling apples into box
                .release       = Led_Release,
            },
};

//insmmod port 
static int Led_DevInit(void)
{
    platform_device_register(&led_dev);
    return 0;
}

static void Led_Release(struct device *dev)
{

}

//remmod port
static void Led_DevExit(void)
{
    platform_device_unregister(&led_dev);
}
module_init(Led_DevInit);
module_exit(Led_DevExit);
MODULE_LICENSE("GPL");