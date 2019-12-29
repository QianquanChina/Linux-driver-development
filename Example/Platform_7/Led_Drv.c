#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include "linux/fs.h"
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/timer.h>
#include <linux/interrupt.h>

/************************************Information*********************************************
*
*		1、利用up控制LED1，down控制LED2，right 控制LED1、LED2全开，left控制LED1、LED2全关
*       2、利用platform机制、顶半部和底半部机制实现的
*       3、按键通过定时器消抖
*		4、提供了ioctl接口可自行编写Test.c 
*
************************************Information**********************************************/

/*---------------------------------------函数声明--------------------------------------------*/
static int         Key_DevInit         (void);
static void        Key_DevExit         (void);

static int         Key_Remove          (struct platform_device *pdev);
static int         Key_Probe           (struct platform_device *pdev);

static int         Led_Probe           (struct platform_device *pdev);
static int         Led_Remove          (struct platform_device *pdev);
static long        Led_Ioctl           (struct file *file, unsigned int cmd, unsigned long buf);
static long        Key_Ioctl           (struct file *file, unsigned int cmd, unsigned long buf);

static irqreturn_t button_isr          (int irq,void *dev);
static void        Key_Tasklet_Fun     (unsigned long date);

static void        Key_Timer_Function  (unsigned long data);
/*---------------------------------------函数声明--------------------------------------------*/

/*---------------------------Led相关资源--------------------------------*/

static void *gpiobase;
static unsigned long *gpiocon;
static unsigned long *gpiodata;
static int  pin;

struct Led_Resource
{
    unsigned long gpio_phys_base;             //寄存器起始地址
    unsigned long size;                       //寄存器的地址空间大小
    unsigned long gpio;                       //GPIO编号

};
static struct platform_driver led_drv = 
{
    .driver = 
            {
                .name = "led_drv"
            },
    .probe  = Led_Probe,                      //匹配成功，内核调用
    .remove = Led_Remove,                     //删除软件或者硬件节点，内核调用
};

static struct file_operations led_fops = 
{
    .unlocked_ioctl = Led_Ioctl
};

static struct miscdevice led_misc =           //注册混杂设备
{
    .minor = MISC_DYNAMIC_MINOR,
    .name  = "myled",
    .fops  = &led_fops,  
};
/*---------------------------Led相关资源--------------------------------*/

/*---------------------------Key相关资源--------------------------------*/
struct Key_Resource
{
    char *name;
    int  gpio ;
    int  code ;
};
static struct platform_driver key_drv = 
{
    .driver = 
            {
                .name = "key_drv"
            },
    .probe  = Key_Probe,                      //匹配成功，内核调用
    .remove = Key_Remove,                     //删除软件或者硬件节点，内核调用
};
static struct file_operations key_fops = 
{
    .unlocked_ioctl = Key_Ioctl
};
static struct miscdevice key_misc =           //定义混杂设备对象
{
    .minor = MISC_DYNAMIC_MINOR,
    .name  = "mykey",
    .fops  = &key_fops,  
};
/*---------------------------Key相关资源--------------------------------*/

/*---------------------------Tasklet相关资源----------------------------*/
static struct tasklet_struct btn_tasklet;     //定义初始化tasklet对象
static int data = 250;
/*---------------------------Tasklet相关资源----------------------------*/


/*---------------------------Timer相关资源------------------------------*/
static struct timer_list key_timer;
static int g_data=250;
/*---------------------------Timer相关资源------------------------------*/

/********************************************************
* @功能: 注册软件节点对象到drv链表
*        遍历、匹配、调用probe等都有内核完成                          
* @参数: 无                                   
* @返值: 0
* @说明: 无
*********************************************************/
static int Key_DevInit(void)
{
    
    platform_driver_register(&key_drv);   
    platform_driver_register(&led_drv);  
    return 0;
}
/********************************************************
* @功能: 没有实际作用                          
* @参数: 无                                   
* @返值: 0
* @说明: 无
*********************************************************/
static long Led_Ioctl(struct file *file, unsigned int cmd, unsigned long buf)
{
    return 0;
}
/********************************************************
* @功能: 没有实际作用                          
* @参数: 无                                   
* @返值: 0
* @说明: 无
*********************************************************/
static long Key_Ioctl(struct file *file, unsigned int cmd, unsigned long buf)
{
    return 0;
}
/********************************************************
* @功能: Led硬件初始化                         
* @参数: 无                                   
* @返值: 0e
* @说明: 无
*********************************************************/
static int Led_Probe(struct platform_device *pdev)
{
    
    struct Led_Resource *pdata = pdev->dev.platform_data;  //获取自定义的硬件信息 结果是 pdata=led_dev.c的&led_info
    //pin = 3;
    pin = pdata->gpio;

    gpiobase = ioremap(pdata->gpio_phys_base, pdata->size);//将寄存器的物理地址映射到内核虚拟地址
    gpiocon  = (unsigned long *)(gpiobase + 0x00);
    gpiodata = (unsigned long *)(gpiobase + 0x04);

    *gpiocon  &= ~(0xFF << (4*pin));                       //gpio 配置模式
    *gpiocon  |= (0x11 << (4*pin));
    *gpiodata &= ~(1 << pin); 
    *gpiodata &= ~(1 << (pin+1) );

    printk("%s\n",__func__);

    misc_register(&led_misc);
    return 0;
}

/********************************************************
* @功能: Key硬件初始化                         
* @参数: 无                                   
* @返值: 0
* @说明: pdev指向匹配成功的节点，For example ：pdev = led_dev.c中的&led_dev
*********************************************************/
static int Key_Probe(struct platform_device *pdev)
{
    unsigned char i,m;
    struct Key_Resource *pdata = pdev->dev.platform_data;   //获取自定义的硬件信息 结果是 pdata=led_dev.c的&led_info

    for ( i = 0; i < 4; i++ )
    {
        int irq = gpio_to_irq( (pdata+i)->gpio);            //通过GPIO请求中断号
        gpio_request((pdata+i)->gpio, (pdata+i)->name);
        m = request_irq(irq, button_isr, IRQF_TRIGGER_FALLING, (pdata+i)->name, (pdata+i) );
    }   
    tasklet_init(&btn_tasklet, Key_Tasklet_Fun, (unsigned long )&data);

    printk("%s！！！\n",__func__);
    misc_register(&key_misc);
    

    init_timer(&key_timer);
    key_timer.function = Key_Timer_Function;
    key_timer.expires  = jiffies+HZ/200;
    key_timer.data     = (unsigned long)&g_data;

    return 0;
}
/********************************************************
* @功能: 中断处理函数                                 
* @参数: irq: 所触发中断的中断号 dev:按键信息                                         
* @返值: 0
* @说明: 无
*********************************************************/
struct Key_Resource *pdata_key;
static irqreturn_t button_isr(int irq,void *dev)
{
    pdata_key = (struct Key_Resource *) dev;
    mod_timer(&key_timer,jiffies+HZ/200);
    return IRQ_HANDLED;
}
/********************************************************
* @功能: 定时器处理函数                                
* @参数: data:外部数值                                         
* @返值: 0
* @说明: 无
*********************************************************/
static void Key_Timer_Function(unsigned long data)
{
    static unsigned char count;
    unsigned char state = 1;
    
    unsigned long flag;
    local_irq_save(flag);

    state = gpio_get_value(pdata_key->gpio);
    if( state == 0)
    {
        count++;                     
        mod_timer(&key_timer,jiffies+HZ/200);
    }else
    {
        count = 0;
        mod_timer(&key_timer,jiffies+HZ/200);
    }

    if(count >= 3)
    {
        del_timer(&key_timer);
        printk("count = %d\n",count);
        count = 0;
        tasklet_schedule(&btn_tasklet);
    }
    local_irq_restore(flag);
}
/********************************************************
* @功能: 延后处理函数  低半部                                
* @参数: data:外部数值                                         
* @返值: 0
* @说明: 无
*********************************************************/
struct Key_Flag 
{
    uint8_t Led1_Flag  : 1;
    uint8_t Led2_Flag  : 1;
};
struct Key_Flag Key = 
{
    .Led1_Flag  = 0,
    .Led2_Flag  = 0,
};
static void Key_Tasklet_Fun(unsigned long date)
{


    switch(pdata_key->code)
    {
    case KEY_UP:
        Key.Led1_Flag = ~Key.Led1_Flag;
        if( Key.Led1_Flag )
        {
            *gpiodata |= (1 << pin);
        }else
        {
            *gpiodata &= ~(1 << pin);
        }
        break;
    case KEY_DOWN:
        Key.Led2_Flag = ~Key.Led2_Flag;
        if( Key.Led2_Flag )
        {
            *gpiodata |= (1 << (pin+1) );
        }else
        {
            *gpiodata &= ~(1 << (pin+1) );
        }   
        break;
    case KEY_LEFT:
        *gpiodata &= ~(1 << (pin+1) );
        *gpiodata &= ~(1 << pin     );

        break;
    case KEY_RIGHT:
        *gpiodata |= (1 << (pin+1) );
        *gpiodata |= (1 << pin     );

        break; 
    default:
        break;
    }
    printk( "%s: 按键[%s], 键值[%d], 状态[]奥里给！！！\n" ,__func__,pdata_key->name, pdata_key->code);

}

/********************************************************
* @功能: 删除Key软件或者硬件，内核自动调用                         
* @参数: struct platform_device *pdev 由Dev传递进来                                  
* @返值: 0
* @说明: pdev指向匹配成功的硬件节点，比如pdev=led_dev.c中的&led_dev
*********************************************************/
static int Key_Remove(struct platform_device *pdev)
{
    unsigned char i;
    struct Key_Resource *pdata = pdev->dev.platform_data;
    for ( i = 0; i < 4; i++ )
    {
        int irq = gpio_to_irq((pdata+i)->gpio);
        gpio_free( (pdata+i)->gpio ); 
        free_irq(irq, (pdata+i)); 
    }
    printk("%s！！！\n",__func__);
    misc_deregister(&key_misc);                           //1.卸载混杂设备
    del_timer(&key_timer);
    return 0;
}
/********************************************************
* @功能: 删除Led软件或者硬件，内核自动调用                        
* @参数: struct platform_device *pdev 由Dev传递进来                                  
* @返值: 0
* @说明: 无
*********************************************************/
static int Led_Remove(struct platform_device *pdev)
{
    
    misc_deregister(&led_misc);                            //1.卸载混杂设备
    *gpiodata &= ~(1 << pin);                              //2.输出0
    iounmap(gpiobase);                                     //3.解除地址映射
    return 0;
}

/********************************************************
* @功能: 删除软件结点                         
* @参数: 无                               
* @返值: 无
* @说明: 无
*********************************************************/
static void Key_DevExit(void)
{
    
    platform_driver_unregister(&led_drv);
    platform_driver_unregister(&key_drv);                   //从drv链表删除软件节点
    return ;
}
module_init(Key_DevInit);
module_exit(Key_DevExit);
MODULE_LICENSE("GPL");