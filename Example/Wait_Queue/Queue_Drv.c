#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
/************************��������************************/
static      int         Btn_Init(void);
static      void        Btn_Exit(void);
static      ssize_t     Btn_Read(struct file *file, char __user *buf,size_t count,loff_t *ppos)       ;
static      ssize_t     Btn_Write(struct file *file, const char __user *buf,size_t count,loff_t *ppos);
/************************��������************************/

/************************ȫ�ֱ���************************/
//����ȴ�����ͷ
static wait_queue_head_t rwq;

/************************ȫ�ֱ���************************/

/********�����ӿ�*********/
static struct file_operations btn_fops = 
{
    .read  = Btn_Read ,
    .write = Btn_Write, 
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
    //1.�����ʼ��װ�����߽��̵�����
    wait_queue_t wait;
    init_waitqueue_entry(&wait,current);
    //2.��ӵ�ǰ���̵��ȴ�������
    add_wait_queue(&rwq, &wait);
    //3.��������λ���ж�����
    set_current_state(TASK_INTERRUPTIBLE);
    //4.��ʽ�������ߵȴ�������ֹͣ��ǰ��ֱ���ý��̱�����
    printk("read process [%s] [%d]will entry sleep status.....\n", current->comm, current->pid);
    schedule();
    printk("Good Man!\n");
    //5.һ�������ѣ�����������Ϊ����
    set_current_state(TASK_RUNNING);
    //6.�����ѵĽ��̴Ӷ������Ƴ�
    remove_wait_queue(&rwq, &wait);
    //7.�жϽ��̱����ѵ�ԭ��
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
* @����:                                  
* @����: ��                                      
* @��ֵ: 1
* @˵��: Write��������ʵ��
*********************************************************/
static ssize_t Btn_Write(struct file *file, const char __user *buf,size_t count,loff_t *ppos)
{
    //����read����
    printk("write process [%s ] [%d] wakeup read process ......\n",current->comm,current->pid); 
    wake_up_interruptible(&rwq);  
    return count;
}
/********************************************************
* @����: ����ж��                                 
* @����: ��                                      
* @��ֵ: ��
* @˵��: ��
*********************************************************/
static void Btn_Exit(void)
{
    misc_deregister(&btn_msic);
}

module_init(Btn_Init);
module_exit(Btn_Exit);
MODULE_LICENSE("GPL");