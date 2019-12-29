#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
/*----------函数声明---------------*/

static int Led_DevInit(void);
static void Led_DevExit(void);
static int Led_Remove(struct platform_device *pdev);
static int Led_Probe(struct platform_device *pdev);
/*----------函数声明---------------*/
static struct platform_driver led_drv = 
{
    .driver = 
            {
                .name = "led_drv"
            },
    .probe  = Led_Probe, //匹配成功，内核调用
    .remove = Led_Remove,//删除软件或者硬件节点，内核调用
};

static int Led_DevInit(void)
{
    //1.注册软件节点对象到drv链表
    //遍历、匹配、调用probe等都有内核完成
    platform_driver_register(&led_drv);   
    return 0;
}
//匹配成功 内核自动调用
//pdev指向匹配成功的节点，For example ：pdev = led_dev.c中的&led_dev
static int Led_Probe(struct platform_device *pdev)
{
    printk("%s\n",__func__);
    return 0;
}
//删除软件或者硬件，内核自动调用
//pdev指向匹配成功的硬件节点，比如pdev=led_dev.c中的&led_dev
static int Led_Remove(struct platform_device *pdev)
{
    printk("%s\n",__func__);
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