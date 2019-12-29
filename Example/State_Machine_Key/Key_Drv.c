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
/**********按键对象***********************************************************/
#define EVENT_CB(ev)   if(handle->cb[ev]) handle->cb[ev]( (Button*)handle )
static struct Button* head_handle = NULL;

#define TICKS_INTERVAL    5	//ms
#define DEBOUNCE_TICKS    3	//MAX 8
#define SHORT_TICKS       (300 /TICKS_INTERVAL)
#define LONG_TICKS        (1000 /TICKS_INTERVAL)


typedef void (*BtnCallback)(void*);

typedef enum {
	PRESS_DOWN = 0, 	//0按键按下，每次按下都触发
	PRESS_UP,			//1按键弹起，每次松开都触发
	PRESS_REPEAT,		//2重复按下触发，变量repeat计数连击次数
	SINGLE_CLICK,		//3单击按键事件
	DOUBLE_CLICK,		//4双击按键事件
	LONG_RRESS_START,	//5达到长按时间阈值时触发一次
	LONG_PRESS_HOLD,	//6长按期间一直触发	
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

/*****************函数声明********************************************/
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
* @功能: Key初始化                                 
* @参数: 无                                            
* @返值: 0
* @说明: 无
*********************************************************/
static struct  file_operations key_fops =       
{
    
};
//定义混杂设备
static struct miscdevice misc_dev = 
{
    //动态分配minor设备号
    .minor = MISC_DYNAMIC_MINOR,
    //open所要打开的驱动
    .name  = "My_State_Machine_Key",
    //文件操作集
    .fops =  &key_fops,
};
//定义定时器
static struct timer_list key_timer;
static int g_data = 251;

//按键对象
struct Button btn1;

static int Key_Init(void)
{
    int i, ret;
    for(i=0; i<ARRAY_SIZE(key_info); i++)
    {
        gpio_request(key_info[i].gpio, key_info[i].name);
        gpio_direction_input(key_info[i].gpio);
    }
    //注册混杂设备
    ret = misc_register(&misc_dev);
    if(ret < 0)
    {
        printk("mise register faild!\n");
    }
    //初始化定时器
    init_timer(&key_timer);
    //定时处理函数功能
    key_timer.function = Key_TimFunction;
    //超时时间5ms
    key_timer.expires  = jiffies + (5/1000)*HZ;
    key_timer.data     = (unsigned long)&g_data;
    //向内核添加定时器，开始倒计时
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
* @功能: 超时处理函数                                 
* @参数: 无                                            
* @返值: 0
* @说明: 无
*********************************************************/
static void Key_TimFunction(unsigned long data)
{
	/************按键扫描*************************/
	struct Button* target;
	for(target=head_handle; target; target=target->next) 
	{
		Button_Handler(target);
	}   
	/************按键扫描*************************/
    //每5ms执行一边此函数
    mod_timer(&key_timer,jiffies+(5/1000)*HZ);
}
/********************************************************
* @功能: 初始化按钮结构句柄                                 
* @参数: handle:按键结构体 pin_level:读取按键状态 
*		 active_level: 按下按键之后是什么状态                                         
* @返值: 无
* @说明: 无
*********************************************************/
void Button_Init(struct Button* handle, uint8_t (*pin_level)(void), uint8_t active_level)
{
	memset(handle, 0, sizeof(struct Button));  //初始化结构体
	handle->event = (uint8_t)NONE_PRESS;	   
	handle->hal_button_Level = pin_level;	   //读取按键状态
	handle->button_level = handle->hal_button_Level(); //将上一步获得的按键此时的状态赋值给hal_button_Level
	handle->active_level = active_level;	   //按键按下之后的状态（根据实际境况而定）
}
/********************************************************
* @功能: 附加按钮事件回调函数                            
* @参数: handle:按键结构体
*		 event:按键事件                                       
* @返值: 无
* @说明: 把相应的处理函数给相应的事件
*********************************************************/
void Button_Attach(struct Button* handle, PressEvent event, BtnCallback cb)
{
	handle->cb[event] = cb;
}

/********************************************************
* @功能: 查询按钮事件的发生                          
* @参数: handle:按键结构体                                     
* @返值: 无
* @说明: 无
*********************************************************/
PressEvent Get_Button_Event(struct Button* handle)
{
	return (PressEvent)(handle->event);
}
/********************************************************
* @功能: 按键驱动核心功能，驱动状态机。                           
* @参数: handle:按键结构体                                     
* @返值: 无
* @说明: 无
*********************************************************/
void (struct Button* handle)
{
	uint8_t read_gpio_level = handle->hal_button_Level();

	//ticks counter working..
	if((handle->state) > 0) handle->ticks++;

	/*------------button debounce handle按钮防反跳处理---------------*/
	//这个函数每5ms进入一次
	//read_gpio_level 获得第一次按下的按键数值
	//handle->button_level再次获得按键的数值 看是否抖动 
	//handle->button_level的初始数值是1 那么如果   （连续）  3次都不等read_gpio_level不等于
	//1 那么说明已经没有抖动了
	/********************************************************************
	*    --------------------------------------------------------------   1  handle->button_level
	*    	/ \  / \ 
	*      /   \/   \-----------------------
	*    --------------------------------------------------------------   0
	******************************************************************/
	//按键按下后read_gpio_level会有波动，如上图所示，当有波动的时候会产生信号0 和 1 当连续3次监测都是0的时候则认为按键已经没有了抖动
	//如果不是连续监测三次中有一次是1那么会进入else 但是else会将handle->debounce_cnt置0 这样就可以保证连续三次监测是 0 
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
	   //此时按键没有了抖动
		handle->debounce_cnt = 0;
	}

	/*-----------------State machine-------------------*/
	//只用通过了237行的赋值才能进行switch里面的if判断 否则 handle->button_level = 1 handle->active_level=0 无法监测的按键是否按下
	switch (handle->state) 
	{
	case 0:
		//按键按下一次
		//判断是否和初始化是给定的按键按下后的值相等，相等则取执行相应的函数
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
		//在第一次按下按键之后会给state = 1 会进入case1 如果监测到释放则给case2执行
		//如果没有监测到按键释放那么是长按 则进入case5进行执行
		//然后判断是否有按键再次按下
		//如果第二次的按键的释放事件超出则会按照单击来算。
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
			//释放超时 比如按键一次后释放 然后进入case2 如果再次监测到 那么就是双击，如果超过了
			//第二次按键的时间 那么就是给当作单击处理。
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
			//这个是双击之后的释放
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
		//进入case5 如果还是按着按键则意味一直保持按键的状态
		//或者是进行了释放
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
* @功能: 启动按钮工作，将句柄添加到工作列表中。                         
* @参数: handle:按键结构体                                     
* @返值: 0 成功 
* @说明: 无
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
* @功能: 停止按钮工作                       
* @参数: handle:按键结构体                                     
* @返值: 0 成功 
* @说明: 无
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
* @功能: 获取按键的状态                     
* @参数: 无                                    
* @返值: 按键的状态 
* @说明: 无
*********************************************************/
uint8_t Read_Button1_GPIO(void) 
{
    return gpio_get_value(key_info[0].gpio);
}

/********************************************************
* @功能: 按键一的函数实现                      
* @参数: 无                                    
* @返值: 无 
* @说明: 无
*********************************************************/
void Button1_Press_Down_Handler(void* btn)
{
	printk("按下\n");
}
void Button1_Press_Up_Handler(void* btn)
{
	printk("松开\n");
}
void Button1_Single_Click_Handler(void *btn)
{
	printk("单击\n");
}
void Button1_Double_Click_Handler(void *btn)
{
	printk("双击\n");
}
void Button1_Long_Press_Start_Handler(void *btn)
{
	printk("长按\n");
}
void Button1_Long_Press_Hold_Handler(void *btn)
{
	printk("保持长按\n");
}
void Button1_Press_Repeat_Handler(void *btn)
{
	printk("Press_Repeat\n");
}
/********************************************************
* @功能: Key释放                                 
* @参数: 无                                            
* @返值: 无
* @说明: 无
*********************************************************/
static void Key_Exit(void)
{
    int i;
    for(i=0; i<ARRAY_SIZE(key_info); i++)
    {
        gpio_free(key_info[i].gpio);        
    }
    misc_deregister(&misc_dev);

    //删除定时器
    del_timer(&key_timer);
}

module_init(Key_Init);
module_exit(Key_Exit);