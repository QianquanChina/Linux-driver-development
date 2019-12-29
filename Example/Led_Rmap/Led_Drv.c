#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/io.h> //ioremap
#include <linux/uaccess.h>
MODULE_LICENSE("GPL");
static unsigned long *gpiocon, *gpiodata;
#define LED_ON  0x100001
#define LED_OFF 0x100002

static int  Led_Init(void);
static void Led_Exit(void);
static long led_ioctl(struct file *file,unsigned int cmd,unsigned long arg);

static struct file_operations led_fops =
{
    .owner = THIS_MODULE,
    .unlocked_ioctl = led_ioctl,
};
static struct miscdevice led_misc = 
{
    .minor = MISC_DYNAMIC_MINOR,
    .name  = "myled",
    .fops  = &led_fops,  
};

static int Led_Init(void)
{
    /*unsigned long *Test;
    unsigned long *Mst;
    void *Mst1;
    Test = gpiocon + 0x01;
    Mst  = Mst1 + 0x01;
    printk("Test = %x , Mst = %x\n",Test, Mst);*/
    misc_register(&led_misc);
    //将物理地址映射到内核的虚拟地址上
    gpiocon = ioremap(0xE0200060, 8);
    //Mst1 = ioremap(0xE0200060, 8);
    //此处相当于 +4 
    printk("gpiocon = %x\n",gpiocon);
    gpiodata = gpiocon +1;
    printk("gpiodata = %x\n",gpiodata);
    //配置GPIO为输出口，输出0
    *gpiocon &= ~( (0xf << 12) | (0xf << 16) );
    *gpiocon |= (1 << 12) | (1<< 16);

    *gpiodata &= ~( (1 <<3 ) | (1 << 4) );

    return 0;
}
static long led_ioctl(struct file *file,
                        unsigned int cmd,
                        unsigned long arg)
{
    int index;
    copy_from_user(&index, (int* )arg, sizeof(index) );
    switch (cmd)
    {
    case LED_ON:
        if(index == 1)
        {
            *gpiodata |= (1 << 3);
            printk("1\n");
        }else 
        if(index == 2)
        {
            *gpiodata |= (1 << 4); 
            printk("2\n");  
        }
        break;
    case LED_OFF:
        if(index == 1)
        {
            *gpiodata &= ~(1 << 3);
            printk("3\n");
        }else
        if(index == 2) 
        {
            *gpiodata &= ~(1 << 4);
            printk("4\n");
        }
        break;
    default:
        break;
    }
        //调试信息
    printk("配置寄存器=%#x, 数据寄存器=%#x\n", *gpiocon, *gpiodata);
    return 0 ;
}

                        
static void Led_Exit(void)
{
    misc_deregister(&led_misc);
    //输出 0 接触地址映射
    *gpiodata &= ~( (1 <<3 ) | (1 << 4) );
    iounmap(gpiocon);
}
module_init(Led_Init);
module_exit(Led_Exit);
