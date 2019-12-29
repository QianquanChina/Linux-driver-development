#include <linux/init.h>
#include <linux/module.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>
#include <linux/input.h>

MODULE_LICENSE("GPL");
/**********��������***********************************************************/
#define EVENT_CB(ev)   if(handle->cb[ev]) handle->cb[ev]( (Button*)handle )
static struct Button* head_handle = NULL;

#define TICKS_INTERVAL    5	//ms
#define DEBOUNCE_TICKS    3	//MAX 8
#define SHORT_TICKS       (300 /TICKS_INTERVAL)
#define LONG_TICKS        (1000 /TICKS_INTERVAL)


typedef void (*BtnCallback)(void*);

typedef enum {
	PRESS_DOWN = 0, 	//0�������£�ÿ�ΰ��¶�����
	PRESS_UP,			//1��������ÿ���ɿ�������
	PRESS_REPEAT,		//2�ظ����´���������repeat������������
	SINGLE_CLICK,		//3���������¼�
	DOUBLE_CLICK,		//4˫�������¼�
	LONG_RRESS_START,	//5�ﵽ����ʱ����ֵʱ����һ��
	LONG_PRESS_HOLD,	//6�����ڼ�һֱ����	
	number_of_event,	//7
	NONE_PRESS			//8
}PressEvent;		

typedef struct Button {
	uint16_t ticks;
	uint8_t  repeat : 4;
	uint8_t  event  : 4;
	uint8_t  state  : 3;
	uint8_t  debounce_cnt : 3; 
	uint8_t  active_level : 1;
	uint8_t  button_level : 1;
	uint8_t  (*hal_button_Level)(void);
	BtnCallback  cb[number_of_event];
	struct Button* next;
}Button;
/*************************************************************************/

/*****************��������********************************************/
static  int         Key_Init                          (void);
static  void        Key_Exit                          (void);
        void        Button_Init                       (struct Button* handle, uint8_t(*pin_level)(void), uint8_t active_level);
        void        Button_Attach                     (struct Button* handle, PressEvent event, BtnCallback cb);
        PressEvent	Get_Button_Event                  (struct Button* handle);
        void        Button_Handler                    (struct Button* handle);
        int         Button_Start     		          (struct Button* handle);
        void        Button_Stop                       (struct Button* handle);
        uint8_t     Read_Button1_GPIO                 (void);
        void        Button1_Press_Down_Handler        (void* btn);
		void 		Button1_Press_Up_Handler          (void* btn);
		void 		Button1_Single_Click_Handler      (void *btn);
		void 		Button1_Double_Click_Handler      (void* btn);
		void 		Button1_Long_Press_Start_Handler  (void* btn);
		void 		Button1_Long_Press_Hold_Handler   (void* btn);
		void    	Button1_Press_Repeat_Handler	  (void *btn);
static  void        Key_TimFunction                   (unsigned long data);				  
/*******************************************************************/
struct Key_Source
{
    char *name;
    int   gpio;
};
static struct Key_Source key_info[] = 
{
    {
        .name  = "Key_Up",
        .gpio  = S5PV210_GPH0(0), 
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
    },
};

/********************************************************
* @����: Key��ʼ��                                 
* @����: ��                                            
* @��ֵ: 0
* @˵��: ��
*********************************************************/
static struct  file_operations key_fops =       
{
    
};
//��������豸
static struct miscdevice misc_dev = 
{
    //��̬����minor�豸��
    .minor = MISC_DYNAMIC_MINOR,
    //open��Ҫ�򿪵�����
    .name  = "My_State_Machine_Key",
    //�ļ�������
    .fops =  &key_fops,
};
//���嶨ʱ��
static struct timer_list key_timer;
static int g_data = 251;

//��������
struct Button btn1;

static int Key_Init(void)
{
    int i, ret;
    for(i=0; i<ARRAY_SIZE(key_info); i++)
    {
        gpio_request(key_info[i].gpio, key_info[i].name);
        gpio_direction_input(key_info[i].gpio);
    }
    //ע������豸
    ret = misc_register(&misc_dev);
    if(ret < 0)
    {
        printk("mise register faild!\n");
    }
    //��ʼ����ʱ��
    init_timer(&key_timer);
    //��ʱ����������
    key_timer.function = Key_TimFunction;
    //��ʱʱ��5ms
    key_timer.expires  = jiffies + (5/1000)*HZ;
    key_timer.data     = (unsigned long)&g_data;
    //���ں���Ӷ�ʱ������ʼ����ʱ
    add_timer(&key_timer);
    printk("%s:begin......\n",__func__); 

	Button_Init(&btn1, Read_Button1_GPIO, 0);
	Button_Attach(&btn1, PRESS_UP        ,  Button1_Press_Up_Handler        );
	Button_Attach(&btn1, PRESS_DOWN      ,  Button1_Press_Down_Handler      );
    Button_Attach(&btn1, PRESS_REPEAT    ,  Button1_Press_Repeat_Handler    );
    Button_Attach(&btn1, SINGLE_CLICK    ,  Button1_Single_Click_Handler    );
    Button_Attach(&btn1, DOUBLE_CLICK    ,  Button1_Double_Click_Handler    );
    Button_Attach(&btn1, LONG_PRESS_HOLD ,  Button1_Long_Press_Hold_Handler );
    Button_Attach(&btn1, LONG_RRESS_START,  Button1_Long_Press_Start_Handler);
	Button_Start(&btn1);  

    return 0;
}
/********************************************************
* @����: ��ʱ������                                 
* @����: ��                                            
* @��ֵ: 0
* @˵��: ��
*********************************************************/
static void Key_TimFunction(unsigned long data)
{
	/************����ɨ��*************************/
	struct Button* target;
	for(target=head_handle; target; target=target->next) 
	{
		Button_Handler(target);
	}   
	/************����ɨ��*************************/
    //ÿ5msִ��һ�ߴ˺���
    mod_timer(&key_timer,jiffies+(5/1000)*HZ);
}
/********************************************************
* @����: ��ʼ����ť�ṹ���                                 
* @����: handle:�����ṹ�� pin_level:��ȡ����״̬ 
*		 active_level: ���°���֮����ʲô״̬                                         
* @��ֵ: ��
* @˵��: ��
*********************************************************/
void Button_Init(struct Button* handle, uint8_t (*pin_level)(void), uint8_t active_level)
{
	memset(handle, 0, sizeof(struct Button));  //��ʼ���ṹ��
	handle->event = (uint8_t)NONE_PRESS;	   
	handle->hal_button_Level = pin_level;	   //��ȡ����״̬
	handle->button_level = handle->hal_button_Level(); //����һ����õİ�����ʱ��״̬��ֵ��hal_button_Level
	handle->active_level = active_level;	   //��������֮���״̬������ʵ�ʾ���������
}
/********************************************************
* @����: ���Ӱ�ť�¼��ص�����                            
* @����: handle:�����ṹ��
*		 event:�����¼�                                       
* @��ֵ: ��
* @˵��: ����Ӧ�Ĵ���������Ӧ���¼�
*********************************************************/
void Button_Attach(struct Button* handle, PressEvent event, BtnCallback cb)
{
	handle->cb[event] = cb;
}

/********************************************************
* @����: ��ѯ��ť�¼��ķ���                          
* @����: handle:�����ṹ��                                     
* @��ֵ: ��
* @˵��: ��
*********************************************************/
PressEvent Get_Button_Event(struct Button* handle)
{
	return (PressEvent)(handle->event);
}
/********************************************************
* @����: �����������Ĺ��ܣ�����״̬����                           
* @����: handle:�����ṹ��                                     
* @��ֵ: ��
* @˵��: ��
*********************************************************/
void (struct Button* handle)
{
	uint8_t read_gpio_level = handle->hal_button_Level();

	//ticks counter working..
	if((handle->state) > 0) handle->ticks++;

	/*------------button debounce handle��ť����������---------------*/
	//�������ÿ5ms����һ��
	//read_gpio_level ��õ�һ�ΰ��µİ�����ֵ
	//handle->button_level�ٴλ�ð�������ֵ ���Ƿ񶶶� 
	//handle->button_level�ĳ�ʼ��ֵ��1 ��ô���   ��������  3�ζ�����read_gpio_level������
	//1 ��ô˵���Ѿ�û�ж�����
	/********************************************************************
	*    --------------------------------------------------------------   1  handle->button_level
	*    	/ \  / \ 
	*      /   \/   \-----------------------
	*    --------------------------------------------------------------   0
	******************************************************************/
	//�������º�read_gpio_level���в���������ͼ��ʾ�����в�����ʱ�������ź�0 �� 1 ������3�μ�ⶼ��0��ʱ������Ϊ�����Ѿ�û���˶���
	//����������������������һ����1��ô�����else ����else�Ὣhandle->debounce_cnt��0 �����Ϳ��Ա�֤�������μ���� 0 
	if(read_gpio_level != handle->button_level) 
	{ //not equal to prev one
		//continue read 3 times same new level change
		if(++(handle->debounce_cnt) >= DEBOUNCE_TICKS) 
		{
			handle->button_level = read_gpio_level;
			handle->debounce_cnt = 0;
		}
	} 
	else 
	{  //leved not change ,counter reset.
	   //��ʱ����û���˶���
		handle->debounce_cnt = 0;
	}

	/*-----------------State machine-------------------*/
	//ֻ��ͨ����237�еĸ�ֵ���ܽ���switch�����if�ж� ���� handle->button_level = 1 handle->active_level=0 �޷����İ����Ƿ���
	switch (handle->state) 
	{
	case 0:
		//��������һ��
		//�ж��Ƿ�ͳ�ʼ���Ǹ����İ������º��ֵ��ȣ������ȡִ����Ӧ�ĺ���
		if(handle->button_level == handle->active_level) //0
		{	//start press down
			handle->event = (uint8_t)PRESS_DOWN;
			EVENT_CB(PRESS_DOWN);
			handle->ticks = 0;
			handle->repeat = 1;
			handle->state = 1;
		} 
		else 
		{
			handle->event = (uint8_t)NONE_PRESS;
		}
		break;
	case 1:
		if(handle->button_level != handle->active_level) 
		{   
			//released press up
			handle->event = (uint8_t)PRESS_UP;
			EVENT_CB(PRESS_UP);
			handle->ticks = 0;
			handle->state = 2;
		}else 
		if(handle->ticks > LONG_TICKS) 
		{
			handle->event = (uint8_t)LONG_RRESS_START;
			EVENT_CB(LONG_RRESS_START);
			handle->state = 5;
		}
		break;
	case 2:
		//�ڵ�һ�ΰ��°���֮����state = 1 �����case1 �����⵽�ͷ����case2ִ��
		//���û�м�⵽�����ͷ���ô�ǳ��� �����case5����ִ��
		//Ȼ���ж��Ƿ��а����ٴΰ���
		//����ڶ��εİ������ͷ��¼�������ᰴ�յ������㡣
		if(handle->button_level == handle->active_level) 
		{   
			//press down again
			handle->event = (uint8_t)PRESS_DOWN;
			EVENT_CB(PRESS_DOWN);
			
			handle->repeat++;
			if(handle->repeat == 2) 
			{
				EVENT_CB(DOUBLE_CLICK); // repeat hit
			} 
			EVENT_CB(PRESS_REPEAT); // repeat hit
			handle->ticks = 0;
			handle->state = 3;
		}else 
		if(handle->ticks > SHORT_TICKS) 
		{ 	
			//�ͷų�ʱ ���簴��һ�κ��ͷ� Ȼ�����case2 ����ٴμ�⵽ ��ô����˫�������������
			//�ڶ��ΰ�����ʱ�� ��ô���Ǹ�������������
			//released timeout
			if(handle->repeat == 1) 
			{
				handle->event = (uint8_t)SINGLE_CLICK;
				EVENT_CB(SINGLE_CLICK);
			}else 
			if(handle->repeat == 2) 
			{
				handle->event = (uint8_t)DOUBLE_CLICK;
			}
			handle->state = 0;
		}
		break;

	case 3:
		if(handle->button_level != handle->active_level) 
		{   
			//�����˫��֮����ͷ�
			//released press up
			handle->event = (uint8_t)PRESS_UP;
			EVENT_CB(PRESS_UP);
			if(handle->ticks < SHORT_TICKS) 
			{
				handle->ticks = 0;
				handle->state = 2; //repeat press
			}
			else 
			{
				handle->state = 0;
			}
		}
		break;
	case 5:
		//����case5 ������ǰ��Ű�������ζһֱ���ְ�����״̬
		//�����ǽ������ͷ�
		if(handle->button_level == handle->active_level) 
		{
			//continue hold trigger
			handle->event = (uint8_t)LONG_PRESS_HOLD;
			EVENT_CB(LONG_PRESS_HOLD);
		} 
		else 
		{ //releasd
			handle->event = (uint8_t)PRESS_UP;
			EVENT_CB(PRESS_UP);
			handle->state = 0; //reset
		}
		break;
	}
}
/********************************************************
* @����: ������ť�������������ӵ������б��С�                         
* @����: handle:�����ṹ��                                     
* @��ֵ: 0 �ɹ� 
* @˵��: ��
*********************************************************/
int Button_Start(struct Button* handle)
{
	struct Button* target = head_handle;
	while(target) 
	{
		if(target == handle) 
		{
			return -1;	//already exist.
		}
		target = target->next;
	}
	handle->next = head_handle;
	head_handle = handle;
	return 0;
}
/********************************************************
* @����: ֹͣ��ť����                       
* @����: handle:�����ṹ��                                     
* @��ֵ: 0 �ɹ� 
* @˵��: ��
*********************************************************/
void Button_Stop(struct Button* handle)
{
	struct Button** curr;
	for(curr = &head_handle; *curr; ) 
	{
		struct Button* entry = *curr;
		if (entry == handle) 
		{
			*curr = entry->next;	
		}
		else
		{
			curr = &entry->next;
		}	
	}
}
/********************************************************
* @����: ��ȡ������״̬                     
* @����: ��                                    
* @��ֵ: ������״̬ 
* @˵��: ��
*********************************************************/
uint8_t Read_Button1_GPIO(void) 
{
    return gpio_get_value(key_info[0].gpio);
}

/********************************************************
* @����: ����һ�ĺ���ʵ��                      
* @����: ��                                    
* @��ֵ: �� 
* @˵��: ��
*********************************************************/
void Button1_Press_Down_Handler(void* btn)
{
	printk("����\n");
}
void Button1_Press_Up_Handler(void* btn)
{
	printk("�ɿ�\n");
}
void Button1_Single_Click_Handler(void *btn)
{
	printk("����\n");
}
void Button1_Double_Click_Handler(void *btn)
{
	printk("˫��\n");
}
void Button1_Long_Press_Start_Handler(void *btn)
{
	printk("����\n");
}
void Button1_Long_Press_Hold_Handler(void *btn)
{
	printk("���ֳ���\n");
}
void Button1_Press_Repeat_Handler(void *btn)
{
	printk("Press_Repeat\n");
}
/********************************************************
* @����: Key�ͷ�                                 
* @����: ��                                            
* @��ֵ: ��
* @˵��: ��
*********************************************************/
static void Key_Exit(void)
{
    int i;
    for(i=0; i<ARRAY_SIZE(key_info); i++)
    {
        gpio_free(key_info[i].gpio);        
    }
    misc_deregister(&misc_dev);

    //ɾ����ʱ��
    del_timer(&key_timer);
}

module_init(Key_Init);
module_exit(Key_Exit);