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
*		1������up����LED1��down����LED2��right ����LED1��LED2ȫ����left����LED1��LED2ȫ��
*       2������platform���ơ����벿�͵װ벿����ʵ�ֵ�
*       3������ͨ����ʱ������
*		4���ṩ��ioctl�ӿڿ����б�дTest.c 
*
************************************Information**********************************************/

/*---------------------------------------��������--------------------------------------------*/
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
/*---------------------------------------��������--------------------------------------------*/

/*---------------------------Led�����Դ--------------------------------*/

static void *gpiobase;
static unsigned long *gpiocon;
static unsigned long *gpiodata;
static int  pin;

struct Led_Resource
{
    unsigned long gpio_phys_base;             //�Ĵ�����ʼ��ַ
    unsigned long size;                       //�Ĵ����ĵ�ַ�ռ��С
    unsigned long gpio;                       //GPIO���

};
static struct platform_driver led_drv = 
{
    .driver = 
            {
                .name = "led_drv"
            },
    .probe  = Led_Probe,                      //ƥ��ɹ����ں˵���
    .remove = Led_Remove,                     //ɾ���������Ӳ���ڵ㣬�ں˵���
};

static struct file_operations led_fops = 
{
    .unlocked_ioctl = Led_Ioctl
};

static struct miscdevice led_misc =           //ע������豸
{
    .minor = MISC_DYNAMIC_MINOR,
    .name  = "myled",
    .fops  = &led_fops,  
};
/*---------------------------Led�����Դ--------------------------------*/

/*---------------------------Key�����Դ--------------------------------*/
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
    .probe  = Key_Probe,                      //ƥ��ɹ����ں˵���
    .remove = Key_Remove,                     //ɾ���������Ӳ���ڵ㣬�ں˵���
};
static struct file_operations key_fops = 
{
    .unlocked_ioctl = Key_Ioctl
};
static struct miscdevice key_misc =           //��������豸����
{
    .minor = MISC_DYNAMIC_MINOR,
    .name  = "mykey",
    .fops  = &key_fops,  
};
/*---------------------------Key�����Դ--------------------------------*/

/*---------------------------Tasklet�����Դ----------------------------*/
static struct tasklet_struct btn_tasklet;     //�����ʼ��tasklet����
static int data = 250;
/*---------------------------Tasklet�����Դ----------------------------*/


/*---------------------------Timer�����Դ------------------------------*/
static struct timer_list key_timer;
static int g_data=250;
/*---------------------------Timer�����Դ------------------------------*/

/********************************************************
* @����: ע������ڵ����drv����
*        ������ƥ�䡢����probe�ȶ����ں����                          
* @����: ��                                   
* @��ֵ: 0
* @˵��: ��
*********************************************************/
static int Key_DevInit(void)
{
    
    platform_driver_register(&key_drv);   
    platform_driver_register(&led_drv);  
    return 0;
}
/********************************************************
* @����: û��ʵ������                          
* @����: ��                                   
* @��ֵ: 0
* @˵��: ��
*********************************************************/
static long Led_Ioctl(struct file *file, unsigned int cmd, unsigned long buf)
{
    return 0;
}
/********************************************************
* @����: û��ʵ������                          
* @����: ��                                   
* @��ֵ: 0
* @˵��: ��
*********************************************************/
static long Key_Ioctl(struct file *file, unsigned int cmd, unsigned long buf)
{
    return 0;
}
/********************************************************
* @����: LedӲ����ʼ��                         
* @����: ��                                   
* @��ֵ: 0e
* @˵��: ��
*********************************************************/
static int Led_Probe(struct platform_device *pdev)
{
    
    struct Led_Resource *pdata = pdev->dev.platform_data;  //��ȡ�Զ����Ӳ����Ϣ ����� pdata=led_dev.c��&led_info
    //pin = 3;
    pin = pdata->gpio;

    gpiobase = ioremap(pdata->gpio_phys_base, pdata->size);//���Ĵ����������ַӳ�䵽�ں������ַ
    gpiocon  = (unsigned long *)(gpiobase + 0x00);
    gpiodata = (unsigned long *)(gpiobase + 0x04);

    *gpiocon  &= ~(0xFF << (4*pin));                       //gpio ����ģʽ
    *gpiocon  |= (0x11 << (4*pin));
    *gpiodata &= ~(1 << pin); 
    *gpiodata &= ~(1 << (pin+1) );

    printk("%s\n",__func__);

    misc_register(&led_misc);
    return 0;
}

/********************************************************
* @����: KeyӲ����ʼ��                         
* @����: ��                                   
* @��ֵ: 0
* @˵��: pdevָ��ƥ��ɹ��Ľڵ㣬For example ��pdev = led_dev.c�е�&led_dev
*********************************************************/
static int Key_Probe(struct platform_device *pdev)
{
    unsigned char i,m;
    struct Key_Resource *pdata = pdev->dev.platform_data;   //��ȡ�Զ����Ӳ����Ϣ ����� pdata=led_dev.c��&led_info

    for ( i = 0; i < 4; i++ )
    {
        int irq = gpio_to_irq( (pdata+i)->gpio);            //ͨ��GPIO�����жϺ�
        gpio_request((pdata+i)->gpio, (pdata+i)->name);
        m = request_irq(irq, button_isr, IRQF_TRIGGER_FALLING, (pdata+i)->name, (pdata+i) );
    }   
    tasklet_init(&btn_tasklet, Key_Tasklet_Fun, (unsigned long )&data);

    printk("%s������\n",__func__);
    misc_register(&key_misc);
    

    init_timer(&key_timer);
    key_timer.function = Key_Timer_Function;
    key_timer.expires  = jiffies+HZ/200;
    key_timer.data     = (unsigned long)&g_data;

    return 0;
}
/********************************************************
* @����: �жϴ�����                                 
* @����: irq: �������жϵ��жϺ� dev:������Ϣ                                         
* @��ֵ: 0
* @˵��: ��
*********************************************************/
struct Key_Resource *pdata_key;
static irqreturn_t button_isr(int irq,void *dev)
{
    pdata_key = (struct Key_Resource *) dev;
    mod_timer(&key_timer,jiffies+HZ/200);
    return IRQ_HANDLED;
}
/********************************************************
* @����: ��ʱ��������                                
* @����: data:�ⲿ��ֵ                                         
* @��ֵ: 0
* @˵��: ��
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
* @����: �Ӻ�����  �Ͱ벿                                
* @����: data:�ⲿ��ֵ                                         
* @��ֵ: 0
* @˵��: ��
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
    printk( "%s: ����[%s], ��ֵ[%d], ״̬[]�����������\n" ,__func__,pdata_key->name, pdata_key->code);

}

/********************************************************
* @����: ɾ��Key�������Ӳ�����ں��Զ�����                         
* @����: struct platform_device *pdev ��Dev���ݽ���                                  
* @��ֵ: 0
* @˵��: pdevָ��ƥ��ɹ���Ӳ���ڵ㣬����pdev=led_dev.c�е�&led_dev
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
    printk("%s������\n",__func__);
    misc_deregister(&key_misc);                           //1.ж�ػ����豸
    del_timer(&key_timer);
    return 0;
}
/********************************************************
* @����: ɾ��Led�������Ӳ�����ں��Զ�����                        
* @����: struct platform_device *pdev ��Dev���ݽ���                                  
* @��ֵ: 0
* @˵��: ��
*********************************************************/
static int Led_Remove(struct platform_device *pdev)
{
    
    misc_deregister(&led_misc);                            //1.ж�ػ����豸
    *gpiodata &= ~(1 << pin);                              //2.���0
    iounmap(gpiobase);                                     //3.�����ַӳ��
    return 0;
}

/********************************************************
* @����: ɾ��������                         
* @����: ��                               
* @��ֵ: ��
* @˵��: ��
*********************************************************/
static void Key_DevExit(void)
{
    
    platform_driver_unregister(&led_drv);
    platform_driver_unregister(&key_drv);                   //��drv����ɾ������ڵ�
    return ;
}
module_init(Key_DevInit);
module_exit(Key_DevExit);
MODULE_LICENSE("GPL");