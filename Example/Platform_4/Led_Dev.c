#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
/*-------------��������-------------*/
static int  Led_DevInit(void);
static void Led_DevExit(void);
static void Led_Release(struct device *dev);
/*-------------��������-------------*/

//�����ʼ������LED��Ӳ����Ϣ����-----Apple
static struct resource led_res[] = 
{
    {
        .start = 0xE0200060,        //start address
        .end   = 0xE0200060 + 0x08, //end   address
        .flags = IORESOURCE_MEM   //address information        
    },
    {
        .start = 3, //GPIO Start number
        .end   = 3,
        .flags = IORESOURCE_IRQ //GPIO Class message 
    }

};

//�����ʼ��LED��Ӳ���ڵ����----Box
static struct platform_device led_dev = 
{
    .name = "led_drv",
    .id   = -1,
    .resource = led_res,//add resource describe hardware information 
    .num_resources = ARRAY_SIZE(led_res),
    .dev = {
                .release = Led_Release
           }
};

//insmmod port 
static int Led_DevInit(void)
{
    //register hardware knot object to dev list
    platform_device_register(&led_dev);
    return 0;
}


static void Led_Release(struct device *dev)
{
    //do nothing
}


//remmod port
static void Led_DevExit(void)
{
    //unregister hardware knot object in dev list
    platform_device_unregister(&led_dev);
}
module_init(Led_DevInit);
module_exit(Led_DevExit);
MODULE_LICENSE("GPL");