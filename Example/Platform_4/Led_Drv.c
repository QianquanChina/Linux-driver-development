#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
/*----------��������---------------*/

static int Led_DevInit(void);
static void Led_DevExit(void);
static int Led_Remove(struct platform_device *pdev);
static int Led_Probe(struct platform_device *pdev);
/*----------��������---------------*/
static struct platform_driver led_drv = 
{
    .driver = 
            {
                .name = "led_drv"
            },
    .probe  = Led_Probe, //ƥ��ɹ����ں˵���
    .remove = Led_Remove,//ɾ���������Ӳ���ڵ㣬�ں˵���
};

static int Led_DevInit(void)
{
    //1.ע������ڵ����drv����
    //������ƥ�䡢����probe�ȶ����ں����
    platform_driver_register(&led_drv);   
    return 0;
}
//ƥ��ɹ� �ں��Զ�����
//pdevָ��ƥ��ɹ��Ľڵ㣬For example ��pdev = led_dev.c�е�&led_dev
static int Led_Probe(struct platform_device *pdev)
{
    printk("%s\n",__func__);
    return 0;
}
//ɾ���������Ӳ�����ں��Զ�����
//pdevָ��ƥ��ɹ���Ӳ���ڵ㣬����pdev=led_dev.c�е�&led_dev
static int Led_Remove(struct platform_device *pdev)
{
    printk("%s\n",__func__);
    return 0;
}
static void Led_DevExit(void)
{
    //��drv����ɾ������ڵ�
    platform_driver_unregister(&led_drv);
}
module_init(Led_DevInit);
module_exit(Led_DevExit);
MODULE_LICENSE("GPL");