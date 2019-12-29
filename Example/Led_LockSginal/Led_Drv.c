#include "linux/miscdevice.h"
#include "linux/module.h"
#include "linux/fs.h"
#include <linux/init.h>
//定义信号量对象
static struct semaphore sema;
/********************************************************
* @功能: Led_Open                                 
* @参数: 无                                      
* @返值: 0 成功 否则失败
* @说明: 无
*********************************************************/
static int Led_Open(struct inode *inode, struct file *file)
{
    printk("request semaphore..\n");
    down(&sema);
    printk("open myled successful....\n");
    return 0;
}
/********************************************************
* @功能: Led_Close                                 
* @参数: 无                                      
* @返值: 0
* @说明: 无
*********************************************************/
static int Led_Close(struct inode *inode, struct file *file)
{
    //释放信号量并且唤醒休眠的进程
    up(&sema);
    return 0;
}
static struct file_operations led_fops = 
{
    .open    = Led_Open,
    .release = Led_Close
};

/*定义混杂设备*/
static struct miscdevice led_misc = 
{
    .minor = MISC_DYNAMIC_MINOR,
    .name  = "myled",
    .fops  = &led_fops
};
/********************************************************
* @功能: 初始化函数                                 
* @参数: 无                                      
* @返值: 1
* @说明: 无
*********************************************************/
static int Led_Init(void)
{
    //混杂设备注册
    misc_register(&led_misc);
    //初始化信号量为互斥信号量
    sema_init(&sema , 1);
    return 0;
}
static void Led_Exit(void)
{
    misc_deregister(&led_misc);
}

module_init(Led_Init);
module_exit(Led_Exit);
MODULE_LICENSE("GPL");