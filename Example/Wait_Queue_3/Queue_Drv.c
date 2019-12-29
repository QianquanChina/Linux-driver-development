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
/************************��������************************/
static      int         Btn_Init(void);
static      void        Btn_Exit(void);
static      ssize_t     Btn_Read(struct file *file, char __user *buf,size_t count,loff_t *ppos);
static      irqreturn_t Btn_Isr(int irq,void *dev);
/************************��������************************/

/************************ȫ�ֱ���************************/
//����ȴ�����ͷ
static wait_queue_head_t rwq;
//��������������Ϣ
struct Btn_Event
{
    int code ;
    int state;
};
struct Btn_Event g_btn;
//������������Ӳ����Ϣ�����ݽṹ
struct Btn_Resource
{
    char *name;
    int   gpio;
    int   code;
};
//�����ʼ��������Ӳ����Ϣ����
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
//���߿���
static int condition=0;
/************************ȫ�ֱ���************************/

/********�����ӿ�*********/
static struct file_operations btn_fops = 
{
    .read  = Btn_Read ,
};

/********ע������豸*****/
static struct miscdevice btn_msic = 
{
    .minor = MISC_DYNAMIC_MINOR,
    .name  = "mybtn"           ,
    .fops  = &btn_fops         ,
};
/********************************************************
* @����: ������ʼ��                                 
* @����: ��                                      
* @��ֵ: 1
* @˵��: ��
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
    //��ʼ���ȴ�����ͷ
    init_waitqueue_head(&rwq);
    return 0;
}
/********************************************************
* @����:                                  
* @����: ��                                      
* @��ֵ: 1
* @˵��: read��������ʵ��
*********************************************************/
static ssize_t Btn_Read(struct file *file, char __user *buf,size_t count,loff_t *ppos)
{
    
    printk("read process [%s] [%d]will entry sleep status.....\n", current->comm, current->pid);
    
    //���� �� ������for(; ;);
    wait_event_interruptible(rwq,condition);
    printk("Good Girl\n");
    //Ϊ����һ��read������
    condition = 0;
    if( 0== (copy_to_user(buf,&g_btn, sizeof(g_btn))) )
    {
        printk("successful\n");
    }
    return count;
}
/********************************************************
* @����:                                  
* @����: irq:���浱ǰ�����жϵ�Ӳ���жϵ��жϺ�
*        dev:���浱ǰ�����жϵ�Ӳ����Ϣ����                                      
* @��ֵ: 1
* @˵��: �жϴ�����ʵ��
*********************************************************/
static irqreturn_t Btn_Isr(int irq,void *dev)
{
    //1.��ȡ������Ӳ����Ϣ
    struct Btn_Resource *pdata = (struct Btn_Resource*)dev;
    //2.��ȡ������״̬�ͼ�ֵ
    g_btn.code  = pdata->code;
    g_btn.state = gpio_get_value(pdata->gpio);
    //3.�������ߵĽ���
    condition = 1;
    wake_up(&rwq);
    return -IRQ_HANDLED;
}
/********************************************************
* @����: ����ж��                                 
* @����: ��                                      
* @��ֵ: ��
* @˵��: ��
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
}

module_init(Btn_Init);
module_exit(Btn_Exit);
MODULE_LICENSE("GPL");