#include <linux/init.h>
#include <linux/module.h>
#include "linux/fs.h"
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#define LED_ON  0x100001
#define LED_OFF 0x100002
/*----------函数声明---------------*/

static int  Led_DevInit(void);
static void Led_DevExit(void);
static long Led_Ioctl(struct file *file, unsigned int cmd, unsigned long buf);
static int  Led_Remove(struct platform_device *pdev);
static int  Led_Probe(struct platform_device *pdev);
/*----------函数声明---------------*/
struct Led_Resource
{
    unsigned long gpio_phys_base;//寄存器起始地址
    unsigned long size;//寄存器的地址空间大小
    unsigned long gpio;//GPIO编号

};
static struct platform_driver led_drv = 
{
    .driver = 
            {
                .name = "led_drv"
            },
    .probe  = Led_Probe, //匹配成功，内核调用
    .remove = Led_Remove,//删除软件或者硬件节点，内核调用
};
//声明描述LED硬件的数据结构
static void *gpiobase;
static unsigned long *gpiocon;
static unsigned long *gpiodata;
static int  pin;

static struct file_operations led_fops = 
{
    .unlocked_ioctl = Led_Ioctl
};
//定义混杂设备对象
static struct miscdevice led_misc = 
{
    .minor = MISC_DYNAMIC_MINOR,
    .name  = "myled",
    .fops  = &led_fops,  
};

static int Led_DevInit(void)
{
    //1.注册软件节点对象到drv链表
    //遍历、匹配、调用probe等都有内核完成
    platform_driver_register(&led_drv);   
    return 0;
}
static long Led_Ioctl(struct file *file, unsigned int cmd, unsigned long buf)
{
    int kindex;
    copy_from_user( &kindex, (int *)buf, sizeof(kindex) );
    //解析指令
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
//匹配成功 内核自动调用
//pdev指向匹配成功的节点，For example ：pdev = led_dev.c中的&led_dev
static int Led_Probe(struct platform_device *pdev)
{
    //获取自定义的硬件信息 结果是 pdata=led_dev.c的&led_info
    struct Led_Resource *pdata = pdev->dev.platform_data;
    //pin = 3;
    pin = pdata->gpio;

    //各种处理
    //将寄存器的物理地址映射到内核虚拟地址
    gpiobase = ioremap(pdata->gpio_phys_base, pdata->size);
    gpiocon  = (unsigned long *)(gpiobase + 0x00);
    gpiodata = (unsigned long *)(gpiobase + 0x04);

    //gpio 配置模式
    *gpiocon  &= ~(0x0F << (4*pin)); 
    *gpiocon  |= (1 << (4*pin));
    *gpiodata &= ~(1 << pin); 
    printk("%s\n",__func__);

    /*这个地方注册混杂设备！！！???*/
    misc_register(&led_misc);
    return 0;
}
//删除软件或者硬件，内核自动调用
//pdev指向匹配成功的硬件节点，比如pdev=led_dev.c中的&led_dev
static int Led_Remove(struct platform_device *pdev)
{
    //1.卸载混杂设备
    misc_deregister(&led_misc);
    //2.输出0
    *gpiodata &= ~(1 << pin);
    //3.解除地址映射
    iounmap(gpiobase);
    return 0;
}
static void Led_DevExit(void)
{
    //从drv链表删除软件节点
    platform_driver_unregister(&led_drv);
}
module_init(Led_DevInit);
module_exit(Led_DevExit);
MODULE_LICENSE("GPL");