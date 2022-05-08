#include "sensor.h"
#include "delay.h"
/*
  本文件主要包括以下内容：
  1.TDS传感器初始化(ADC)
  2.PH计传感器初始化(ADC)
  3.超声波传感器初始化（普通端口与TIM定时器）——>用于检测垃圾仓剩余容量
  4.电池电量初始化（ADC）
  5.电池剩余电量百分比检测
  6.垃圾仓容量百分比检测
*/

/*
  端口定义
#define TDS_port 	AD_Value[0]		PC0
#define PH_port  	AD_Value[1]		PC1
*/
#define TRIG_Send  PBout(6) 
#define ECHO_Reci  PBin(7)
/*
  函数定义
*/
#define  M    2
volatile float AD_Value[M];
u16 msHcCount = 0;//ms计数
static int voltage=100;
u32 voltage_count=0;

void ADC_init()
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_InitTypeDef       ADC_InitStructure;
	DMA_InitTypeDef  DMA_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);//DMA1时钟使能

	//模拟口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;//PC0,PC1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//模拟输入
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;//不带上下拉
	GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化 

	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1,ENABLE);	  //ADC1复位
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1,DISABLE);	//复位结束	
	
	//DMA设置  
    //DMA通道配置  
    DMA_DeInit(DMA2_Stream0);  
    DMA_InitStructure.DMA_Channel = DMA_Channel_0;   
    //外设地址  
    DMA_InitStructure.DMA_PeripheralBaseAddr = ((u32)ADC1+0x4c);  //(uint32_t)(&ADC1->DR)
    //内存地址  
    DMA_InitStructure.DMA_Memory0BaseAddr =(u32)&AD_Value;  //DMA内存基地址  
    //dma传输方向  
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;  
    //设置DMA在传输时缓冲区的长度  
    DMA_InitStructure.DMA_BufferSize = M;  //DMA通道的DMA缓存的大小	M*N
    //设置DMA的外设递增模式，一个外设  
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  
    //设置DMA的内存递增模式  
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  
    //外设数据字长  
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  
    //内存数据字长  
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;  
    //设置DMA的传输模式  
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  
    //设置DMA的优先级别  
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;  
      
    //指定如果FIFO模式或直接模式将用于指定的流:不使能FIFO模式    
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;      
    //指定了FIFO阈值水平  
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;          
    //指定的Burst转移配置内存传输   
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;         
    //指定的Burst转移配置外围转移  
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;   
      
    //配置DMA的通道           
    DMA_Init(DMA2_Stream0, &DMA_InitStructure); 
    //使能通道  
    DMA_Cmd(DMA2_Stream0, ENABLE);  	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); //使能ADC1时钟
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1,ENABLE);    
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1,DISABLE); 
	//初始化ADC  
    ADC_DeInit();  

	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;//独立模式
	ADC_CommonInitStructure.ADC_DMAAccessMode =  ADC_DMAAccessMode_Disabled; //DMA失能，//关闭DMA存取  
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;//预分频4分频。ADCCLK=PCLK2/4=84/4=21Mhz,ADC时钟最好不要超过36Mhz 	
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;  //两次采样间隔  
	ADC_CommonInit(&ADC_CommonInitStructure);//初始化
	
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;//12位模式
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;//扫描模式	
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//连续转换
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//禁止触发检测 
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//右对齐	
	ADC_InitStructure.ADC_NbrOfConversion = M;//1个转换在规则序列中 也就是只转换规则序列1 
	ADC_Init(ADC1, &ADC_InitStructure);//ADC初始化
	
	//规则模式通道配置 
	ADC_RegularChannelConfig(ADC1,ADC_Channel_11,1, ADC_SampleTime_56Cycles);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_12,2, ADC_SampleTime_56Cycles);
	
	//使能ADC的DMA 
	ADC_DMARequestAfterLastTransferCmd(ADC1,ENABLE); //源数据变化时开启DMA传输
	ADC_DMACmd(ADC1,ENABLE);//使能ADC的DMA传输
	 
	//使能ADC1 
	ADC_Cmd(ADC1, ENABLE);//开启AD转换器	
	
	//开启ADC1的软件转换
	ADC_SoftwareStartConv(ADC1);
}
//tips：static函数的作用域仅限于定义它的源文件内，所以不需要在头文件里声明
static void OpenTimerForHc()        //打开定时器
{
    TIM_SetCounter(TIM7,0);//清除计数
    msHcCount = 0;
    TIM_Cmd(TIM7, ENABLE);  //使能TIMx外设
}
 
static void CloseTimerForHc()        //关闭定时器
{
    TIM_Cmd(TIM7, DISABLE);  //使能TIMx外设
}
void HCSR04_init()
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;     //生成用于定时器设置的结构体
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE); //使能GPIOB的时钟
    //IO初始化
    GPIO_InitStructure.GPIO_Pin =GPIO_Pin_6;       //发送电平引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;        //输出功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//速度100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //推挽输出

    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB,GPIO_Pin_6);
     
    GPIO_InitStructure.GPIO_Pin =   GPIO_Pin_7;     //返回电平引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //输出功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//速度100MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);  
    GPIO_ResetBits(GPIOB,GPIO_Pin_7);    
     
    //定时器初始化 使用基本定时器TIM6
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);   //使能对应RCC时钟
    //配置定时器基础结构体
    TIM_DeInit(TIM7);
    TIM_TimeBaseStructure.TIM_Period = (1000-1); //设置在下一个更新事件装入活动的自动重装载寄存器周期的值         计数到1000为1ms
    TIM_TimeBaseStructure.TIM_Prescaler =(84-1); //设置用来作为TIMx时钟频率除数的预分频值  1M的计数频率 1US计数
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;//不分频
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
    TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位         
        
    TIM_ClearFlag(TIM7, TIM_FLAG_Update);   //清除更新中断，免得一打开中断立即产生中断
    TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE);    //打开定时器更新中断
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;             //选择串口1中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //抢占式中断优先级设置为1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;         //响应式中断优先级设置为1
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;        //使能中断
    NVIC_Init(&NVIC_InitStructure);
	
    TIM_Cmd(TIM7,DISABLE);     
}
//获取定时器时间
u32 GetEchoTimer(void)
{
    u32 t = 0;
    t = msHcCount*1000;//得到MS
    t += TIM_GetCounter(TIM7);//得到US
    TIM7->CNT = 0;  //将TIM2计数寄存器的计数值清零
    delay_ms(50);
    return t;
}
//一次获取超声波测距数据 两次测距之间需要相隔一段时间，隔断回响信号
//为了消除余震的影响，取五次数据的平均值进行加权滤波。
float Hcsr04GetLength(void)
{
    u32 t = 0;
    int i = 0;
    float lengthTemp = 0;
    float sum = 0;
    while(i!=5)
    {
        TRIG_Send = 1;      //发送口高电平输出
        delay_us(20);
        TRIG_Send = 0;
        while(ECHO_Reci == 0);      //等待接收口高电平输出
        OpenTimerForHc();        //打开定时器
        i = i + 1;
        while(ECHO_Reci == 1);
        CloseTimerForHc();        //关闭定时器
        t = GetEchoTimer();        //获取时间,分辨率为1US
        lengthTemp = ((float)t/58.0);//cm
        sum = lengthTemp + sum ;
    }
    lengthTemp = sum/5.0;
    return lengthTemp;
}
void TIM7_IRQHandler(void)   //TIM7中断
{
    if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
    {
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update);  //清除TIMx更新中断标志 
        msHcCount++;
		voltage_count++;
		if(voltage_count >= 120000000)		//每隔2分钟，显示电量百分比减少1
		{
			voltage = voltage - 1;		
		}
    }
}
void AllSensor_init()
{
	ADC_init();
	HCSR04_init();
}
float current_tds()
{
	float tds;
	tds = AD_Value[0];
	return tds;
}
int current_ph()
{
	float ph;
	ph = AD_Value[1];
	return ph;
}
int margin_storage_percent()
{
	int storage;
	storage = (int)(Hcsr04GetLength()*0.05);
	return storage;
}
int margin_voltage_percent()
{
	return voltage;
}
