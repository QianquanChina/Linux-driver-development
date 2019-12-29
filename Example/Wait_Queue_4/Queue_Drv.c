#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/input.h>
/************************函数声明************************/
static  int         Btn_Init        (void);
static  void        Btn_Exit        (void);
static  ssize_t     Btn_Read        (struct file *file, char __user *buf,size_t count,loff_t *ppos);
static  irqreturn_t Btn_Isr         (int irq, void *dev);
static  void        Key_TimFunction (unsigned long data);	
/************************函数声明************************/

/************************全局变量************************/
//声明描述按键信息
struct Btn_Event
{
    int code ;
    int state;
};
//声明描述按键硬件信息的数据结构
struct Btn_Resource
{
    char *name;
    int   gpio;
    int   code;
};
//定义初始胡按键的硬件信息对象
static struct Btn_Resource btn_info[]  = 
{
    {
        .name = "KEY_UP"       ,
        .gpio = S5PV210_GPH0(0),
        .code = KEY_UP         ,
    },
    {
        .name = "KEY_DOWN"     ,
        .gpio = S5PV210_GPH0(1),
        .code = KEY_DOWN       ,
    },
    {
        .name = "KEY_LEFT"     ,
        .gpio = S5PV210_GPH0(2),
        .code = KEY_LEFT       ,
    },
    {
        .name = "KEY_RIGHT"    ,
        .gpio = S5PV210_GPH0(3),
        .code = KEY_RIGHT      ,
    }
};
struct Btn_Event g_btn;
//定义等待队列头
static wait_queue_head_t rwq;
//休眠开关
static int condition=0;
//定义定时器对象
static struct timer_list btn_timer;
//
//保存当前触发的按键的硬件信息,给超时处理函数使用
//中断处理函数赋值
struct Btn_Resource *pdata=NULL;
/************************全局变量************************/

/********函数接口*********/
static struct file_operations btn_fops = 
{
    .read  = Btn_Read ,
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
    int i;
    for(i=0; i<ARRAY_SIZE(btn_info); i++)
    {
        int irq = gpio_to_irq(btn_info[i].gpio);
        gpio_request(btn_info[i].gpio, btn_info[i].name);
        request_irq(irq, Btn_Isr, IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING,  btn_info[i].name, &btn_info[i]);

    }
    misc_register(&btn_msic);
    //初始化等待队列头
    init_waitqueue_head(&rwq);
    //初始话定时器
    init_timer(&btn_timer);
    //定时处理函数功能
    btn_timer.function = Key_TimFunction;
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
    
    printk("read process [%s] [%d]will entry sleep status.....\n", current->comm, current->pid);
    
    //休眠 宏 本质是for(; ;);
    wait_event_interruptible(rwq,condition);
    printk("Good Girl and Good Man\n");
    //为了下一次read能休眠
    condition = 0;
    if( 0== (copy_to_user(buf,&g_btn, sizeof(g_btn))) )
    {
        printk("successful\n");
    }
    return count;
}
/********************************************************
* @功能:                                  
* @参数: irq:保存当前触发中断的硬件中断的中断号
*        dev:保存当前触发中断的硬件信息对象                                      
* @返值: 1
* @说明: 中断处理函数实现
*********************************************************/
static irqreturn_t Btn_Isr(int irq,void *dev)
{
    //1.获取按键的硬件信息
    pdata = (struct Btn_Resource*)dev;
    //修改定时器
    mod_timer(&btn_timer,jiffies+msecs_to_jiffies(10));
    return -IRQ_HANDLED;
}
/********************************************************
* @功能:                               
* @参数:                                    
* @返值: 1
* @说明: 定时器超时函数实现
*********************************************************/
static void Key_TimFunction (unsigned long data)
{
    //2.获取按键的状态和键值
    g_btn.code  = pdata->code;
    g_btn.state = gpio_get_value(pdata->gpio);
    condition=1;//为了read进程从wait宏中返回
    wake_up(&rwq);
}
/********************************************************
* @功能: 驱动卸载                                 
* @参数: 无                                      
* @返值: 无
* @说明: 无
*********************************************************/
static void Btn_Exit(void)
{
    int i;
    for(i=0; i<ARRAY_SIZE(btn_info); i++)
    {
        int irq=gpio_to_irq(btn_info[i].gpio);
        gpio_free(btn_info[i].gpio);
        free_irq(irq, &btn_info[i]);
    }
    misc_deregister(&btn_msic);
    //删除定时器
    del_timer(&btn_timer);
}

module_init(Btn_Init);
module_exit(Btn_Exit);
MODULE_LICENSE("GPL");