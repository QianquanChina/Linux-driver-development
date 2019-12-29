#include "linux/init.h"
#include "linux/module.h"
#include "linux/gpio.h"
#include "linux/cdev.h"
#include "linux/fs.h"
#include "linux/uaccess.h"
#include <linux/miscdevice.h>
MODULE_LICENSE("GPL");

static ssize_t Key_Read(struct file *file,char __user * buf, size_t count,loff_t * ppos);

struct Key_Source
{
    char *name;
    int  gpio;
};
static struct Key_Source key_info[] = 
{
    {
        .name = "Key_Up",
        .gpio = S5PV210_GPH0(0),
    },
    {
        .name = "Key_Down",
        .gpio = S5PV210_GPH0(1),
    },    
    {
        .name = "Key_Left",
        .gpio = S5PV210_GPH0(2),
    },
    {
        .name = "Key_Right",
        .gpio = S5PV210_GPH0(3),
    }
};
static struct file_operations key_fops = 
{
    .read = Key_Read,
};
static struct miscdevice misc_dev = 
{
    //��̬����minor�豸��
    .minor = MISC_DYNAMIC_MINOR,
    //open��Ҫ�򿪵�����
    .name  = "mykey",
    //�ļ�������
    .fops =  &key_fops,
};
/********************************************************
* @����: Key��ʼ��                                 
* @����: struct file *file : fd 
*        char __user * buf : �û�������
*        size_t count      : �����ֽڴ�С
*        loff_t * ppos     : �ļ�ƫ����                                       
* @��ֵ: ��ȡ�ֽڵ�����
* @˵��: ��
*********************************************************/
struct key_state
{
    int kstate[4];
    int index[4];
};
struct key_state key;
static ssize_t Key_Read(struct file *file,char __user * buf, size_t count,loff_t * ppos)
{
    int i;
    for ( i = 0; i < ARRAY_SIZE(key_info); i++ )
    {
        key.kstate[i] = gpio_get_value(key_info[i].gpio);
        key.index[i]      = i+1;

        if ( copy_to_user( buf, &key, sizeof(key) ) )
        {
            printk("copy filde\n");
        }
    }
    return count;
}
/********************************************************
* @����: Key��ʼ��                                 
* @����: ��                                            
* @��ֵ: 0
* @˵��: ��
*********************************************************/
static int Key_Init(void)
{
    unsigned char i, ret;

    for ( i = 0; i < ARRAY_SIZE(key_info); i++ )
    {
        gpio_request(key_info[i].gpio, key_info[i].name);
        gpio_direction_input(key_info[i].gpio);
    }
    ret = misc_register(&misc_dev);
    if( ret < 0 )
    {
        printk("misc_register fail\n");
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
        gpio_free(key_info[i].gpio);  
    }
    misc_deregister(&misc_dev);
}

module_init(Key_Init);
module_exit(Key_Exit);

