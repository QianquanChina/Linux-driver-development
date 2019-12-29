#include "linux/gpio.h"
#include "linux/miscdevice.h"
#include "linux/module.h"
#include "linux/fs.h"
#include <linux/init.h>
//�����ʼ������ԭ�ӱ�����ֵΪ1
// static atomic_t open_cnt = 1 �������кܴ��������Ȼ���ǳ�ʼ��Ϊ1
static atomic_t open_cnt = ATOMIC_INIT(1); //������Դ
/********************************************************
* @����: Led_Open                                 
* @����: ��                                      
* @��ֵ: 0 �ɹ� ����ʧ��
* @˵��: ��
*********************************************************/
static int Led_Open(struct inode *inode, struct file *file)
{
    //��ȡ��
    //atomic_dec_and_test(&open_cnt) �Լ��������ԡ��Ƿ�Ϊ 0
    //0 ����True ���� ���� False
    //��һ�η���ֵ�� 0 �� ����True ȡ��֮��ͳ� Flase ���޷�
    //����if ����ĺ�������򿪳ɹ�
    //�ڶ����Լ��������ֲ���0 �򷵻�False ȡ��֮��True ��ô
    //����ִ��if����ĺ��� ����豸ʧ�ܣ�ʵ���˻������
    if(!atomic_dec_and_test(&open_cnt))
    {
        printk("open my led failed......\n");
        atomic_inc(&open_cnt);
        return -EBUSY;
    }

    printk("open successful....\n");
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
    atomic_inc(&open_cnt); //open_cnt ++ �ͷ���Դ

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
     return 0;
}
static void Led_Exit(void)
{
    misc_deregister(&led_misc);
}

module_init(Led_Init);
module_exit(Led_Exit);
MODULE_LICENSE("GPL");