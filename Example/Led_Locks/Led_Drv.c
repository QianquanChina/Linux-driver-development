#include "linux/gpio.h"
#include "linux/miscdevice.h"
#include "linux/module.h"
#include "linux/fs.h"
#include <linux/init.h>
static int open_cnt = 1; //共享资源
static spinlock_t lock;  //自旋锁定义
/********************************************************
* @功能: Led_Open                                 
* @参数: 无                                      
* @返值: 0 成功 否则失败
* @说明: 无
*********************************************************/
static int Led_Open(struct inode *inode, struct file *file)
{
    //获取锁
    spin_lock(&lock);
    //临界区  if(--open_cnt !=0)
    if(--open_cnt != 0)
    {
        printk("open my led failed......\n");
        open_cnt++;
        spin_unlock(&lock);
        return -EBUSY;
    }
    spin_unlock(&lock);
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
    //获取锁
    spin_lock(&lock);
    open_cnt++;
    spin_unlock(&lock);
    return 0;
}
static struct file_operations led_fops = 
{
    .open  = Led_Open,
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
    //初始化自旋锁
    spin_lock_init(&lock);
    return 0;
}
static void Led_Exit(void)
{
    misc_deregister(&led_misc);
}

module_init(Led_Init);
module_exit(Led_Exit);
MODULE_LICENSE("GPL");