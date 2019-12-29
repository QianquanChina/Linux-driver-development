#include "linux/miscdevice.h"
#include "linux/module.h"
#include "linux/fs.h"
#include <linux/init.h>
//�����ź�������
static struct semaphore sema;
/********************************************************
* @����: Led_Open                                 
* @����: ��                                      
* @��ֵ: 0 �ɹ� ����ʧ��
* @˵��: ��
*********************************************************/
static int Led_Open(struct inode *inode, struct file *file)
{
    printk("request semaphore..\n");
    down(&sema);
    printk("open myled successful....\n");
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
    //�ͷ��ź������һ������ߵĽ���
    up(&sema);
    return 0;
}
static struct file_operations led_fops = 
{
    .open    = Led_Open,
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
    //��ʼ���ź���Ϊ�����ź���
    sema_init(&sema , 1);
    return 0;
}
static void Led_Exit(void)
{
    misc_deregister(&led_misc);
}

module_init(Led_Init);
module_exit(Led_Exit);
MODULE_LICENSE("GPL");