#include "linux/init.h"
#include "linux/module.h"
#include "linux/gpio.h"
#include "linux/cdev.h"
#include "linux/fs.h"
#include "linux/uaccess.h"
#include "linux/device.h"
MODULE_LICENSE("GPL");
struct Led_Flag 
{
    char Time_Flag : 1;
};
struct Led_Flag led_flag = 
{
    .Time_Flag = 0,
};

//led的灯资源
struct Led_Resoure
{
    char *name;
    int   gpio;
};
static struct Led_Resoure led_info[] = 
{
    {
        .name = "Frist_Led",
        .gpio = S5PV210_GPC0(3),
    },
    {
        .name = "Secend_Led",
        .gpio = S5PV210_GPC0(4),    
    }
};
static struct cdev led_cdev;
static dev_t dev;


//定义定时器对象
static struct timer_list led_timer;
static int g_data = 251;

/********************************************************
* @功能: 定时器处理函数                                 
* @参数: unsigned long 初始化定时器获得才参数                                       
* @返值: 无
* @说明: 此参数用指针 因为定义的是int 有可能是16或者32
*********************************************************/
static void Led_TimFunction(unsigned long data)
{
    int i;
    printk("%s:g_date = %d\n",__func__, *( (int*)data ) );
    led_flag.Time_Flag = ~(led_flag.Time_Flag);
    if(led_flag.Time_Flag == 0)
    {
        for(i=0; i<ARRAY_SIZE(led_info); i++)
        {
            gpio_set_value(led_info[i].gpio, 1);
        }
        
    }else
    {
        for(i=0; i<ARRAY_SIZE(led_info); i++)
        {
            gpio_set_value(led_info[i].gpio, 0);
        }    
    } 
    /*
    * 第一种方法，如果在 1 2两句之间触发了中断 当定时器超时，此超时处理函数获得CPU资源进行处理
    * 当CPU刚执行完led_timer.expires = jiffies + 2*HZ; 触发硬件中断，当硬件中断处理完毕之后，
    * CPU内核再次回到超时处理函数继续运行add_timer(&led_timer);，本来向设置超时时间间隔2S后执行
    * 此函，但是硬件处理函数会消耗20s 那么
    *
    */
    #if 0
    //1修改超时时间
    led_timer.expires = jiffies + 2*HZ;
    //2重新添加定时器.PS:如果不添加那么此函数就执行一次，类似于Zigbee的协议栈
    add_timer(&led_timer);
    #endif
    #if 1
    mod_timer(&led_timer,jiffies+2*HZ);
    //等价于：del_timer+expires....+add_timer
    //此函数内核帮你完成互斥访问，能够让CPU踏踏实实的
    //执行三步骤，不会有中断来抢夺CPU资源。    
    #endif

}

/********************************************************
* @功能: Led_初始化函数                                 
* @参数: 无                                         
* @返值: int  0
* @说明: 无
*********************************************************/
static int Led_Init(void)
{
    int i;
    for(i=0; i<ARRAY_SIZE(led_info); i++)
    {
        gpio_request(led_info[i].gpio, led_info[i].name);
        gpio_direction_output(led_info[i].gpio, 1);
    }
    //申请设备号 不用申请 但是需要申请gpio的资源
    alloc_chrdev_region(&dev, 0, 1, "tarena");
    printk("major:  %d, minor:  %d",MAJOR(dev),MINOR(dev));
    //不需要添加硬件操作接口

    cdev_add(&led_cdev,dev,1);
    /*
    *定时器相关操作
    */
    init_timer(&led_timer);
    //定时处理函数功能
    led_timer.function = Led_TimFunction;
    //超时时间
    led_timer.expires  = jiffies + 2*HZ;
    led_timer.data     = (unsigned long)&g_data;
    //向内核添加定时器，开始倒计时
    add_timer(&led_timer);
    printk("%s:begin......\n",__func__);
    return 0;
}
static void Led_Exit(void)
{
    int i;
    //从内核中卸载字符设备对象
    cdev_del(&led_cdev);
    //释放设备号
    unregister_chrdev_region(dev, 1);
    //输出0释放GPIO资源
    for(i=0; i<ARRAY_SIZE(led_info); i++)
    {
        gpio_set_value(led_info[i].gpio,0);
        gpio_free(led_info[i].gpio);
    }
    /*
    *删除定时器
    */
    del_timer(&led_timer);
}
module_init(Led_Init);
module_exit(Led_Exit);

