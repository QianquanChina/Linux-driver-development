#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/mm.h> //mmap
#include <linux/uaccess.h>
MODULE_LICENSE("GPL");

static int  Led_Init(void);
static void Led_Exit(void);
static int Led_Mmap(struct file *file, struct vm_area_struct *vma);

static struct file_operations led_fops =
{
    .mmap = Led_Mmap
};
static struct miscdevice led_misc = 
{
    .minor = MISC_DYNAMIC_MINOR,
    .name  = "myled",
    .fops  = &led_fops,  
};

static int Led_Init(void)
{
    misc_register(&led_misc);
    return 0;
}
//vam 指向内核创建的va_area_struct对象，描述空闲用户虚拟内存的属性
static int Led_Mmap(struct file *file, struct vm_area_struct *vma) 
{
    
    //1.为了防止出现数据访问不一致性问题
    //只需将cache关闭
    vma->vm_page_prot=pgprot_noncached(vma->vm_page_prot);
    //将物理地址和用户虚拟地址映射
    //空闲虚拟内存对象  //已知的用户虚拟内存的起始地址 //已知的物理地址 //大小 //读写权限
    //0xE0200060 0xE0200000 猜测：此时映射的的地址起始地址的一个页面
    remap_pfn_range(vma, vma->vm_start, 0xE0200000 >> 12, vma->vm_end-vma->vm_start, vma->vm_page_prot);
    return 0;
}   

static void Led_Exit(void)
{
    misc_deregister(&led_misc);
}
module_init(Led_Init);
module_exit(Led_Exit);
