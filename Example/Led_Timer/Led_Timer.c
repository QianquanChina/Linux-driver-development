#include "linux/init.h"
#include "linux/module.h"
#include "linux/gpio.h"
#include "linux/cdev.h"
#include "linux/fs.h"
#include "linux/uaccess.h"
#include "linux/device.h"
MODULE_LICENSE("GPL");
struct Led_Flag 
{
    char Time_Flag : 1;
};
struct Led_Flag led_flag = 
{
    .Time_Flag = 0,
};

//led�ĵ���Դ
struct Led_Resoure
{
    char *name;
    int   gpio;
};
static struct Led_Resoure led_info[] = 
{
    {
        .name = "Frist_Led",
        .gpio = S5PV210_GPC0(3),
    },
    {
        .name = "Secend_Led",
        .gpio = S5PV210_GPC0(4),    
    }
};
static struct cdev led_cdev;
static dev_t dev;


//���嶨ʱ������
static struct timer_list led_timer;
static int g_data = 251;

/********************************************************
* @����: ��ʱ��������                                 
* @����: unsigned long ��ʼ����ʱ����òŲ���                                       
* @��ֵ: ��
* @˵��: �˲�����ָ�� ��Ϊ�������int �п�����16����32
*********************************************************/
static void Led_TimFunction(unsigned long data)
{
    int i;
    printk("%s:g_date = %d\n",__func__, *( (int*)data ) );
    led_flag.Time_Flag = ~(led_flag.Time_Flag);
    if(led_flag.Time_Flag == 0)
    {
        for(i=0; i<ARRAY_SIZE(led_info); i++)
        {
            gpio_set_value(led_info[i].gpio, 1);
        }
        
    }else
    {
        for(i=0; i<ARRAY_SIZE(led_info); i++)
        {
            gpio_set_value(led_info[i].gpio, 0);
        }    
    } 
    /*
    * ��һ�ַ���������� 1 2����֮�䴥�����ж� ����ʱ����ʱ���˳�ʱ���������CPU��Դ���д���
    * ��CPU��ִ����led_timer.expires = jiffies + 2*HZ; ����Ӳ���жϣ���Ӳ���жϴ������֮��
    * CPU�ں��ٴλص���ʱ��������������add_timer(&led_timer);�����������ó�ʱʱ����2S��ִ��
    * �˺�������Ӳ��������������20s ��ô
    *
    */
    #if 0
    //1�޸ĳ�ʱʱ��
    led_timer.expires = jiffies + 2*HZ;
    //2������Ӷ�ʱ��.PS:����������ô�˺�����ִ��һ�Σ�������Zigbee��Э��ջ
    add_timer(&led_timer);
    #endif
    #if 1
    mod_timer(&led_timer,jiffies+2*HZ);
    //�ȼ��ڣ�del_timer+expires....+add_timer
    //�˺����ں˰�����ɻ�����ʣ��ܹ���CPṲ̤ʵʵ��
    //ִ�������裬�������ж�������CPU��Դ��    
    #endif

}

/********************************************************
* @����: Led_��ʼ������                                 
* @����: ��                                         
* @��ֵ: int  0
* @˵��: ��
*********************************************************/
static int Led_Init(void)
{
    int i;
    for(i=0; i<ARRAY_SIZE(led_info); i++)
    {
        gpio_request(led_info[i].gpio, led_info[i].name);
        gpio_direction_output(led_info[i].gpio, 1);
    }
    //�����豸�� �������� ������Ҫ����gpio����Դ
    alloc_chrdev_region(&dev, 0, 1, "tarena");
    printk("major:  %d, minor:  %d",MAJOR(dev),MINOR(dev));
    //����Ҫ���Ӳ�������ӿ�

    cdev_add(&led_cdev,dev,1);
    /*
    *��ʱ����ز���
    */
    init_timer(&led_timer);
    //��ʱ����������
    led_timer.function = Led_TimFunction;
    //��ʱʱ��
    led_timer.expires  = jiffies + 2*HZ;
    led_timer.data     = (unsigned long)&g_data;
    //���ں���Ӷ�ʱ������ʼ����ʱ
    add_timer(&led_timer);
    printk("%s:begin......\n",__func__);
    return 0;
}
static void Led_Exit(void)
{
    int i;
    //���ں���ж���ַ��豸����
    cdev_del(&led_cdev);
    //�ͷ��豸��
    unregister_chrdev_region(dev, 1);
    //���0�ͷ�GPIO��Դ
    for(i=0; i<ARRAY_SIZE(led_info); i++)
    {
        gpio_set_value(led_info[i].gpio,0);
        gpio_free(led_info[i].gpio);
    }
    /*
    *ɾ����ʱ��
    */
    del_timer(&led_timer);
}
module_init(Led_Init);
module_exit(Led_Exit);

