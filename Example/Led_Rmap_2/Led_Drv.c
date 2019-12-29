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
//vam ָ���ں˴�����va_area_struct�������������û������ڴ������
static int Led_Mmap(struct file *file, struct vm_area_struct *vma) 
{
    
    //1.Ϊ�˷�ֹ�������ݷ��ʲ�һ��������
    //ֻ�轫cache�ر�
    vma->vm_page_prot=pgprot_noncached(vma->vm_page_prot);
    //�������ַ���û������ַӳ��
    //���������ڴ����  //��֪���û������ڴ����ʼ��ַ //��֪�������ַ //��С //��дȨ��
    //0xE0200060 0xE0200000 �²⣺��ʱӳ��ĵĵ�ַ��ʼ��ַ��һ��ҳ��
    remap_pfn_range(vma, vma->vm_start, 0xE0200000 >> 12, vma->vm_end-vma->vm_start, vma->vm_page_prot);
    return 0;
}   

static void Led_Exit(void)
{
    misc_deregister(&led_misc);
}
module_init(Led_Init);
module_exit(Led_Exit);
