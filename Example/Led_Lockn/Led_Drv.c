#include "linux/miscdevice.h"
#include "linux/module.h"
#include "linux/fs.h"
#include <linux/init.h>
static int open_cnt = 1; //������Դ
/********************************************************
* @����: Led_Open                                 
* @����: ��                                      
* @��ֵ: 0 �ɹ� ����ʧ��
* @˵��: ��
*********************************************************/
static int Led_Open(struct inode *inode, struct file *file)
{
    //�ٽ���  if(--open_cnt !=0)
    if(--open_cnt != 0)
    {
        printk("open my led failed......\n");
        open_cnt++;
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
    open_cnt++;
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