#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
/*-------------函数声明-------------*/
static int Led_DevInit(void);
static void Led_DevExit(void);
static void Led_Release(struct device *dev);
/*-------------函数声明-------------*/

//声明描述LED硬件信息的数据结构
struct Led_Resource
{
    /* data */
    unsigned long gpio_phys_base;   //寄存器起始地址
    unsigned long size;             //寄存器的地址大小空间
    unsigned long gpio;             //GPIO编号
};

//定义初始化描述LED的硬件信息对象-----Apple
static struct Led_Resource led_info = 
{
    .gpio_phys_base = 0xE0200060,
    .size           = 8,
    .gpio           = 3,
};

//定义初始化LED的硬件节点对象----Box
static struct platform_device led_dev = 
{
    .name = "led_drv",
    .id   = -1,
    .dev  = {
                .platform_data = &led_info, //装载硬件信息 Fling apples into box
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