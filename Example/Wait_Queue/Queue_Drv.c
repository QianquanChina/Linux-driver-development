#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
/************************函数声明************************/
static      int         Btn_Init(void);
static      void        Btn_Exit(void);
static      ssize_t     Btn_Read(struct file *file, char __user *buf,size_t count,loff_t *ppos)       ;
static      ssize_t     Btn_Write(struct file *file, const char __user *buf,size_t count,loff_t *ppos);
/************************函数声明************************/

/************************全局变量************************/
//定义等待队列头
static wait_queue_head_t rwq;

/************************全局变量************************/

/********函数接口*********/
static struct file_operations btn_fops = 
{
    .read  = Btn_Read ,
    .write = Btn_Write, 
};

/********注册混杂设备*****/
static struct miscdevice btn_msic = 
{
    .minor = MISC_DYNAMIC_MINOR,
    .name  = "mybtn"           ,
    .fops  = &btn_fops         ,
};
/********************************************************
* @功能: 驱动初始化                                 
* @参数: 无                                      
* @返值: 1
* @说明: 无
*********************************************************/
static int Btn_Init(void)
{
    misc_register(&btn_msic);
    //初始化等待队列头
    init_waitqueue_head(&rwq);
    return 0;
}
/********************************************************
* @功能:                                  
* @参数: 无                                      
* @返值: 1
* @说明: read驱动函数实现
*********************************************************/
static ssize_t Btn_Read(struct file *file, char __user *buf,size_t count,loff_t *ppos)
{
    //1.定义初始化装做休眠进程的容器
    wait_queue_t wait;
    init_waitqueue_entry(&wait,current);
    //2.添加当前进程到等待队列中
    add_wait_queue(&rwq, &wait);
    //3.设置休眠位可中断类型
    set_current_state(TASK_INTERRUPTIBLE);
    //4.正式进入休眠等待，代码停止不前，直到该进程被唤醒
    printk("read process [%s] [%d]will entry sleep status.....\n", current->comm, current->pid);
    schedule();
    printk("Good Man!\n");
    //5.一旦被唤醒，将进程设置为运行
    set_current_state(TASK_RUNNING);
    //6.将唤醒的进程从队列中移除
    remove_wait_queue(&rwq, &wait);
    //7.判断进程被唤醒的原因
    if(signal_pending(current))
    {
        printk("read process[%s] [%d],receive kill signal....\n",current->comm,current->pid);
        return -ERESTARTSYS;
    }else
    {
        printk("driver wakeup process [%s ] [%d] ......\n", current->comm,current->pid);
    }
    return count;
}
/********************************************************
* @功能:                                  
* @参数: 无                                      
* @返值: 1
* @说明: Write驱动函数实现
*********************************************************/
static ssize_t Btn_Write(struct file *file, const char __user *buf,size_t count,loff_t *ppos)
{
    //唤醒read进程
    printk("write process [%s ] [%d] wakeup read process ......\n",current->comm,current->pid); 
    wake_up_interruptible(&rwq);  
    return count;
}
/********************************************************
* @功能: 驱动卸载                                 
* @参数: 无                                      
* @返值: 无
* @说明: 无
*********************************************************/
static void Btn_Exit(void)
{
    misc_deregister(&btn_msic);
}

module_init(Btn_Init);
module_exit(Btn_Exit);
MODULE_LICENSE("GPL");