#include <linux/init.h>
#include <linux/module.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>
#include <linux/input.h>
MODULE_LICENSE("GPL");

struct Key_Source
{
    char *name;
    int  gpio;
    int code;
};
static struct Key_Source key_info[] = 
{
    {
        .name = "Key_Up",
        .gpio = S5PV210_GPH0(0),
        .code=KEY_UP,
    },
    {
        .name = "Key_Down",
        .gpio = S5PV210_GPH0(1),
        .code=KEY_DOWN,
    },    
    {
        .name = "Key_Left",
        .gpio = S5PV210_GPH0(2),
        .code=KEY_LEFT,
    },
    {
        .name = "Key_Right",
        .gpio = S5PV210_GPH0(3),
        .code=KEY_RIGHT,
    }
};
/********************************************************
* @功能: 延后处理函数  低半部  //软中断原理                             
* @参数: date：外部数值                                        
* @返值: void
* @说明: 无
*********************************************************/
//定义初始化tasklet对象
static struct tasklet_struct btn_tasklet;
//底半部 不紧急 不能休眠 
//CPU在合适的时候执行
static struct Key_Source *pdata;

static void Btn_Tasklet_Fun(unsigned long date)
{
    //获取按键的状态
    int state = gpio_get_value(pdata->gpio);
    printk("%s : 按键[%s],键值[%d], 状态[%s]\n",__func__, pdata->name, pdata->code, state?"按下":"松开");
}
/********************************************************
* @功能: 中断处理函数   顶半部                              
* @参数: irq: 所触发中断的中断号 dev:按键信息                                         
* @返值: 0
* @说明: 无
*********************************************************/
static irqreturn_t button_isr(int irq,void *dev)
{
    //通过dev获取当前触发的按键的硬件信息对象
    pdata = (struct Key_Source *) dev;
    tasklet_schedule(&btn_tasklet);
    return IRQ_HANDLED;
}

/********************************************************
* @功能: Key初始化                                 
* @参数: 无                                            
* @返值: 0
* @说明: 无
*********************************************************/
static int data = 250;
static int Key_Init(void)
{
    unsigned char i;

    for ( i = 0; i < ARRAY_SIZE(key_info); i++ )
    {
        ///通过GPIO请求中断号
        int irq = gpio_to_irq(key_info[i].gpio);
        gpio_request(key_info[i].gpio, key_info[i].name);
        request_irq(irq, button_isr, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, key_info[i].name, &key_info[i] );
    }
    //初始化延后处理函数
    tasklet_init(&btn_tasklet, Btn_Tasklet_Fun, (unsigned long *)&data);
    return 0;
}
/********************************************************
* @功能: Key释放                                 
* @参数: 无                                            
* @返值: 无
* @说明: 无
*********************************************************/
static void Key_Exit(void)
{
    unsigned char i;

    for ( i = 0; i < ARRAY_SIZE(key_info); i++ )
    {
        int irq = gpio_to_irq(key_info[i].gpio);
        gpio_free(key_info[i].gpio); 
        free_irq(irq, &key_info[i]); 
    }
    
}
module_init(Key_Init);
module_exit(Key_Exit);

