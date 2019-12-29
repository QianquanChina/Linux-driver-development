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
static  int         Btn_Init        (void);
static  void        Btn_Exit        (void);
static  ssize_t     Btn_Read        (struct file *file, char __user *buf,size_t count,loff_t *ppos);
static  irqreturn_t Btn_Isr         (int irq, void *dev);
static  void        Key_TimFunction (unsigned long data);	
/************************��������************************/

/************************ȫ�ֱ���************************/
//��������������Ϣ
struct Btn_Event
{
    int code ;
    int state;
};
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
struct Btn_Event g_btn;
//����ȴ�����ͷ
static wait_queue_head_t rwq;
//���߿���
static int condition=0;
//���嶨ʱ������
static struct timer_list btn_timer;
//
//���浱ǰ�����İ�����Ӳ����Ϣ,����ʱ������ʹ��
//�жϴ�������ֵ
struct Btn_Resource *pdata=NULL;
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
    //��ʼ����ʱ��
    init_timer(&btn_timer);
    //��ʱ����������
    btn_timer.function = Key_TimFunction;
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
    printk("Good Girl and Good Man\n");
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
    pdata = (struct Btn_Resource*)dev;
    //�޸Ķ�ʱ��
    mod_timer(&btn_timer,jiffies+msecs_to_jiffies(10));
    return -IRQ_HANDLED;
}
/********************************************************
* @����:                               
* @����:                                    
* @��ֵ: 1
* @˵��: ��ʱ����ʱ����ʵ��
*********************************************************/
static void Key_TimFunction (unsigned long data)
{
    //2.��ȡ������״̬�ͼ�ֵ
    g_btn.code  = pdata->code;
    g_btn.state = gpio_get_value(pdata->gpio);
    condition=1;//Ϊ��read���̴�wait���з���
    wake_up(&rwq);
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
    //ɾ����ʱ��
    del_timer(&btn_timer);
}

module_init(Btn_Init);
module_exit(Btn_Exit);
MODULE_LICENSE("GPL");