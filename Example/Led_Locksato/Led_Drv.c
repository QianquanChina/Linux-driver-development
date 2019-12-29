#include "linux/gpio.h"
#include "linux/miscdevice.h"
#include "linux/module.h"
#include "linux/fs.h"
#include <linux/init.h>
//定义初始化整形原子变量的值为1
// static atomic_t open_cnt = 1 和下面有很大的区别虽然都是初始化为1
static atomic_t open_cnt = ATOMIC_INIT(1); //共享资源
/********************************************************
* @功能: Led_Open                                 
* @参数: 无                                      
* @返值: 0 成功 否则失败
* @说明: 无
*********************************************************/
static int Led_Open(struct inode *inode, struct file *file)
{
    //获取锁
    //atomic_dec_and_test(&open_cnt) 自减操作测试。是否为 0
    //0 返回True 否则 返回 False
    //第一次返回值是 0 则 返回True 取反之后就成 Flase 在无法
    //进行if 里面的函数，则打开成功
    //第二次自减操作发现不是0 则返回False 取反之后True 那么
    //将会执行if里面的函数 则打开设备失败，实现了互斥访问
    if(!atomic_dec_and_test(&open_cnt))
    {
        printk("open my led failed......\n");
        atomic_inc(&open_cnt);
        return -EBUSY;
    }

    printk("open successful....\n");
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
    atomic_inc(&open_cnt); //open_cnt ++ 释放资源

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
     return 0;
}
static void Led_Exit(void)
{
    misc_deregister(&led_misc);
}

module_init(Led_Init);
module_exit(Led_Exit);
MODULE_LICENSE("GPL");