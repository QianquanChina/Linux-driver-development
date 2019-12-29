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
*	                                硬件资源信息	
*      
************************************Information**********************************************/

/*---------------------函数声明---------------------*/
static int  Key_DevInit  (void);
static void Key_DevExit  (void);

static void Key_Release  (struct device *dev);
static void Led_Release  (struct device *dev);
/*---------------------函数声明---------------------*/


/*--------声明描述LED硬件信息的数据结构以及定义Led信息对象以及初始化LED硬件节点信息-------*/

struct Led_Resource                            //声明描述LED硬件信息的数据结构
{
    unsigned long gpio_phys_base;              //寄存器起始地址
    unsigned long size;                        //寄存器的地址大小空间
    unsigned long gpio;                        //GPIO编号
};                               
static struct Led_Resource led_info =          //定义初始化描述LED的硬件信息对象-----Apple
{
    .gpio_phys_base = 0xE0200060,
    .size           = 8,
    .gpio           = 3,
};
static struct platform_device led_dev =        //定义初始化LED的硬件节点对象----Box
{
    .name = "led_drv",
    .id   = -1,
    .dev  = {
                .platform_data = &led_info,    //装载硬件信息 Fling apples into box
                .release       = Led_Release,
            },
};

/*--------声明描述LED硬件信息的数据结构以及定义Led信息对象以及初始化LED硬件节点信息-------*/


/*--------声明描述KEY硬件信息的数据结构以及定义Key信息对象以及初始化Key硬件节点信息-------*/

struct Key_Resource                            //声明描述Key硬件信息的数据结构
{
    char *name;
    int  gpio;
    int  code;
};
static struct Key_Resource key_info[] =        //定义初始化描述Key的硬件信息对象-----Apple
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
static struct platform_device key_dev =        //定义初始化Key的硬件节点对象----Box
{
    .name = "key_drv",
    .id   = -1,
    .dev  = {
                .platform_data = key_info,    //装载硬件信息 Fling apples into box
                .release       = Key_Release,
            },                
};
/*--------声明描述KEY硬件信息的数据结构以及定义Key信息对象以及初始化Key硬件节点信息-------*/


/********************************************************
* @功能: 初始化                                 
* @参数: 无                                   
* @返值: 0
* @说明: 无
*********************************************************/
static int Key_DevInit(void)
{
    platform_device_register(&led_dev);
    platform_device_register(&key_dev);
    return 0;
}

/********************************************************
* @功能: 消除警告终端输出 OK                                 
* @参数: 无                                   
* @返值: 无
* @说明: 无
*********************************************************/
static void Key_Release(struct device *dev)
{
    printk("OK!\n");

    return;
}

/********************************************************
* @功能: 消除警告终端输出 OK                                  
* @参数: 无                                   
* @返值: 无
* @说明: 无
*********************************************************/
static void Led_Release(struct device *dev)
{
    printk("OK_Successful!\n");

    return;
}

/********************************************************
* @功能: 卸载                                 
* @参数: 无                                   
* @返值: 无
* @说明: 无
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