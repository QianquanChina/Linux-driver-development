#include "linux/gpio.h"
#include "linux/miscdevice.h"
#include "linux/module.h"
#include "linux/fs.h"
#include <linux/init.h>
static int open_cnt = 1; //������Դ
static spinlock_t lock;  //��������������
/********************************************************
* @����: Led_Open                                 
* @����: ��                                      
* @��ֵ: 0 �ɹ� ����ʧ��
* @˵��: ��
*********************************************************/
static int Led_Open(struct inode *inode, struct file *file)
{
    //��ȡ��
    unsigned long flags; //���ж������ִ�������������� ����Ļָ�
    //������жϺ��������Ľ��
    spin_lock_irqsave(&lock,flags);
    //�ٽ���  if(--open_cnt !=0)
    if(--open_cnt != 0)
    {
        printk("open my led failed......\n");
        open_cnt++;
        spin_unlock_irqrestore(&lock,flags);
        return -EBUSY;
    }
    spin_unlock_irqrestore(&lock,flags);
    printk("open successful\n");
    return 0;
}
/********************************************************
* @����: Led_Close                                 
* @����: ��                                      
* @��ֵ: 0
* @˵��: ��
*********************************************************/
static int Led_Close(struct inode *inode, struct file *file)
{
    //��ȡ��
    unsigned long flags; //���ж������ִ�������������� ����Ļָ�
    spin_lock_irqsave(&lock,flags);
    open_cnt++;
    spin_unlock_irqrestore(&lock,flags);
    return 0;
}
static struct file_operations led_fops = 
{
    .open  = Led_Open,
    .release = Led_Close
};

/*��������豸*/
static struct miscdevice led_misc = 
{
    .minor = MISC_DYNAMIC_MINOR,
    .name  = "myled",
    .fops  = &led_fops
};
/********************************************************
* @����: ��ʼ������                                 
* @����: ��                                      
* @��ֵ: 1
* @˵��: ��
*********************************************************/
static int Led_Init(void)
{
    //�����豸ע��
    misc_register(&led_misc);
    //��ʼ��������
    spin_lock_init(&lock);
    return 0;
}
static void Led_Exit(void)
{
    misc_deregister(&led_misc);
}

module_init(Led_Init);
module_exit(Led_Exit);
MODULE_LICENSE("GPL");