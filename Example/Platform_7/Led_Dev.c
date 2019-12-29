#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include "linux/cdev.h"
#include "linux/uaccess.h"

/************************************Information*********************************************
*
*	                                Ӳ����Դ��Ϣ	
*      
************************************Information**********************************************/

/*---------------------��������---------------------*/
static int  Key_DevInit  (void);
static void Key_DevExit  (void);

static void Key_Release  (struct device *dev);
static void Led_Release  (struct device *dev);
/*---------------------��������---------------------*/


/*--------��������LEDӲ����Ϣ�����ݽṹ�Լ�����Led��Ϣ�����Լ���ʼ��LEDӲ���ڵ���Ϣ-------*/

struct Led_Resource                            //��������LEDӲ����Ϣ�����ݽṹ
{
    unsigned long gpio_phys_base;              //�Ĵ�����ʼ��ַ
    unsigned long size;                        //�Ĵ����ĵ�ַ��С�ռ�
    unsigned long gpio;                        //GPIO���
};                               
static struct Led_Resource led_info =          //�����ʼ������LED��Ӳ����Ϣ����-----Apple
{
    .gpio_phys_base = 0xE0200060,
    .size           = 8,
    .gpio           = 3,
};
static struct platform_device led_dev =        //�����ʼ��LED��Ӳ���ڵ����----Box
{
    .name = "led_drv",
    .id   = -1,
    .dev  = {
                .platform_data = &led_info,    //װ��Ӳ����Ϣ Fling apples into box
                .release       = Led_Release,
            },
};

/*--------��������LEDӲ����Ϣ�����ݽṹ�Լ�����Led��Ϣ�����Լ���ʼ��LEDӲ���ڵ���Ϣ-------*/


/*--------��������KEYӲ����Ϣ�����ݽṹ�Լ�����Key��Ϣ�����Լ���ʼ��KeyӲ���ڵ���Ϣ-------*/

struct Key_Resource                            //��������KeyӲ����Ϣ�����ݽṹ
{
    char *name;
    int  gpio;
    int  code;
};
static struct Key_Resource key_info[] =        //�����ʼ������Key��Ӳ����Ϣ����-----Apple
{
    {
        .name = "Key_Up",
        .gpio = S5PV210_GPH0(0),
        .code = KEY_UP,
    },
    {
        .name = "Key_Down",
        .gpio = S5PV210_GPH0(1),
        .code = KEY_DOWN,
    },    
    {
        .name = "Key_Left",
        .gpio = S5PV210_GPH0(2),
        .code = KEY_LEFT,
    },
    {
        .name = "Key_Right",
        .gpio = S5PV210_GPH0(3),
        .code = KEY_RIGHT,
    }
};
static struct platform_device key_dev =        //�����ʼ��Key��Ӳ���ڵ����----Box
{
    .name = "key_drv",
    .id   = -1,
    .dev  = {
                .platform_data = key_info,    //װ��Ӳ����Ϣ Fling apples into box
                .release       = Key_Release,
            },                
};
/*--------��������KEYӲ����Ϣ�����ݽṹ�Լ�����Key��Ϣ�����Լ���ʼ��KeyӲ���ڵ���Ϣ-------*/


/********************************************************
* @����: ��ʼ��                                 
* @����: ��                                   
* @��ֵ: 0
* @˵��: ��
*********************************************************/
static int Key_DevInit(void)
{
    platform_device_register(&led_dev);
    platform_device_register(&key_dev);
    return 0;
}

/********************************************************
* @����: ���������ն���� OK                                 
* @����: ��                                   
* @��ֵ: ��
* @˵��: ��
*********************************************************/
static void Key_Release(struct device *dev)
{
    printk("OK!\n");

    return;
}

/********************************************************
* @����: ���������ն���� OK                                  
* @����: ��                                   
* @��ֵ: ��
* @˵��: ��
*********************************************************/
static void Led_Release(struct device *dev)
{
    printk("OK_Successful!\n");

    return;
}

/********************************************************
* @����: ж��                                 
* @����: ��                                   
* @��ֵ: ��
* @˵��: ��
*********************************************************/
static void Key_DevExit(void)
{
    platform_device_unregister(&led_dev);
    platform_device_unregister(&key_dev);
    return;
}
module_init(Key_DevInit);
module_exit(Key_DevExit);
MODULE_LICENSE("GPL");