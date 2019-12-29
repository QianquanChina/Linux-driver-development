#include "linux/init.h"
#include "linux/module.h"
#include "linux/gpio.h"
#include "linux/cdev.h"
#include "linux/fs.h"
#include "linux/uaccess.h"
static long Led_Ioctl(struct file *file, unsigned int cmd, unsigned long buf);

//�Զ�����������
//fd0->file0->inode0->i_rdev0->���豸�ţ����豸��0
//fd1->file1->inode1->i_rdev1->���豸�ţ����豸��1
MODULE_LICENSE("GPL");
#define LED_OPEN  0x10001
#define LED_CLOSE 0x10002
struct Led_Resource 
{
    char *name;
    int   gpio;
};

static struct Led_Resource led_info[] =
{
    {
        .name = "Frist_LED",
        .gpio = S5PV210_GPC0(3),
    },
    {
        .name = "Secend_LED",
        .gpio = S5PV210_GPC0(4),
        
    }
};
static struct  file_operations led_fops =       
{
    .unlocked_ioctl = Led_Ioctl,
};

static struct  cdev led_cdev;
static dev_t dev;

static int Led_Init(void)
{
    int i;
    //����GPIO��Դ������Ϊ��� �������Ϊ0
    for(i=0; i<ARRAY_SIZE(led_info); i++)
    {
        gpio_request(led_info[i].gpio,led_info[i].name);
        gpio_direction_output(led_info[i].gpio, 1);
    }
    //�����豸��
    alloc_chrdev_region(&dev, 0, 2, "tarena");
    printk("major:  %d, minor:  %d",MAJOR(dev),MINOR(dev));

    //��ʼ���豸��������豸Ӳ�������ӿ�
    cdev_init(&led_cdev, &led_fops);

    //���ں���ע���ַ��豸����
    //�� cdev_init()������ʼ���ѷ��䵽�Ľṹ���� file_operations �ṹ����������
    //������ cdev_add()�������豸���� struct  cdev �ṹ���й��������ں���ʽ��
    //�����豸��ע�ᣬ�������豸���Ա��������ˡ�
    cdev_add(&led_cdev,dev,2);
    return 0;
}
static void Led_Exit(void)
{
    int i;
    
    //���ں���ж���ַ��豸����
    cdev_del(&led_cdev);

    //�ͷ��豸��
    unregister_chrdev_region(dev, 2);
    //���0�ͷ�GPIO��Դ
    for(i=0; i<ARRAY_SIZE(led_info); i++)
    {
        gpio_set_value(led_info[i].gpio,0);
        gpio_free(led_info[i].gpio);
    }

}

static long Led_Ioctl(struct file *file, unsigned int cmd, unsigned long buf)
{

    //ͨ�����豸������ ͨ��file���ҵ���Ӧ��inode
    struct  inode *inode = file->f_path.dentry->d_inode;
    //��ô��豸����
    int mimor = MINOR(inode->i_rdev);
    switch (cmd)
    {
        
    case LED_OPEN:
        gpio_set_value(led_info[mimor].gpio, 1);
        printk("%s:turn open %d led.\n",__func__,mimor+1);
        break;
    case LED_CLOSE:
        gpio_set_value(led_info[mimor].gpio, 0);
        printk("%s:turn close %d led.\n",__func__,mimor+1);
        break;
    default:
        printk("command is invalid\n");
        break;
    }
    return 0;
}

module_init(Led_Init);
module_exit(Led_Exit);