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
    int  code;
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
* @����: �жϴ�����                                 
* @����: irq: �������жϵ��жϺ� dev:������Ϣ                                         
* @��ֵ: 0
* @˵��: ��
*********************************************************/
static irqreturn_t button_isr(int irq,void *dev)
{
    //ͨ��dev��ȡ��ǰ�����İ�����Ӳ����Ϣ����
    struct Key_Source * pdata = (struct Key_Source *) dev;
    //��ȡ����״̬
    int state = gpio_get_value(pdata->gpio);
    printk( "%s: ����[%s], ��ֵ[%d], ״̬[%s]\n" ,__func__,pdata->name, pdata->code, state?"����":"�ɿ�");
    return IRQ_HANDLED;
}

/********************************************************
* @����: Key��ʼ��                                 
* @����: ��                                            
* @��ֵ: 0
* @˵��: ��
*********************************************************/
static int Key_Init(void)
{
    unsigned char i;

    for ( i = 0; i < ARRAY_SIZE(key_info); i++ )
    {
        ///ͨ��GPIO�����жϺ�
        int irq = gpio_to_irq(key_info[i].gpio);
        gpio_request(key_info[i].gpio, key_info[i].name);
        request_irq(irq, button_isr, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, key_info[i].name, &key_info[i] );
    }
    return 0;
}
/********************************************************
* @����: Key�ͷ�                                 
* @����: ��                                            
* @��ֵ: ��
* @˵��: ��
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

