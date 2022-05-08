#include "sensor.h"
#include "delay.h"
/*
  ���ļ���Ҫ�����������ݣ�
  1.TDS��������ʼ��(ADC)
  2.PH�ƴ�������ʼ��(ADC)
  3.��������������ʼ������ͨ�˿���TIM��ʱ��������>���ڼ��������ʣ������
  4.��ص�����ʼ����ADC��
  5.���ʣ������ٷֱȼ��
  6.�����������ٷֱȼ��
*/

/*
  �˿ڶ���
#define TDS_port 	AD_Value[0]		PC0
#define PH_port  	AD_Value[1]		PC1
*/
#define TRIG_Send  PBout(6) 
#define ECHO_Reci  PBin(7)
/*
  ��������
*/
#define  M    2
volatile float AD_Value[M];
u16 msHcCount = 0;//ms����
static int voltage=100;
u32 voltage_count=0;

void ADC_init()
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_InitTypeDef       ADC_InitStructure;
	DMA_InitTypeDef  DMA_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);//DMA1ʱ��ʹ��

	//ģ�������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;//PC0,PC1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//ģ������
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;//����������
	GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ�� 

	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1,ENABLE);	  //ADC1��λ
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1,DISABLE);	//��λ����	
	
	//DMA����  
    //DMAͨ������  
    DMA_DeInit(DMA2_Stream0);  
    DMA_InitStructure.DMA_Channel = DMA_Channel_0;   
    //�����ַ  
    DMA_InitStructure.DMA_PeripheralBaseAddr = ((u32)ADC1+0x4c);  //(uint32_t)(&ADC1->DR)
    //�ڴ��ַ  
    DMA_InitStructure.DMA_Memory0BaseAddr =(u32)&AD_Value;  //DMA�ڴ����ַ  
    //dma���䷽��  
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;  
    //����DMA�ڴ���ʱ�������ĳ���  
    DMA_InitStructure.DMA_BufferSize = M;  //DMAͨ����DMA����Ĵ�С	M*N
    //����DMA���������ģʽ��һ������  
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  
    //����DMA���ڴ����ģʽ  
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  
    //���������ֳ�  
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  
    //�ڴ������ֳ�  
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;  
    //����DMA�Ĵ���ģʽ  
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  
    //����DMA�����ȼ���  
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;  
      
    //ָ�����FIFOģʽ��ֱ��ģʽ������ָ������:��ʹ��FIFOģʽ    
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;      
    //ָ����FIFO��ֵˮƽ  
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;          
    //ָ����Burstת�������ڴ洫��   
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;         
    //ָ����Burstת��������Χת��  
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;   
      
    //����DMA��ͨ��           
    DMA_Init(DMA2_Stream0, &DMA_InitStructure); 
    //ʹ��ͨ��  
    DMA_Cmd(DMA2_Stream0, ENABLE);  	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); //ʹ��ADC1ʱ��
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1,ENABLE);    
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1,DISABLE); 
	//��ʼ��ADC  
    ADC_DeInit();  

	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;//����ģʽ
	ADC_CommonInitStructure.ADC_DMAAccessMode =  ADC_DMAAccessMode_Disabled; //DMAʧ�ܣ�//�ر�DMA��ȡ  
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;//Ԥ��Ƶ4��Ƶ��ADCCLK=PCLK2/4=84/4=21Mhz,ADCʱ����ò�Ҫ����36Mhz 	
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;  //���β������  
	ADC_CommonInit(&ADC_CommonInitStructure);//��ʼ��
	
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;//12λģʽ
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;//ɨ��ģʽ	
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//����ת��
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//��ֹ������� 
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//�Ҷ���	
	ADC_InitStructure.ADC_NbrOfConversion = M;//1��ת���ڹ��������� Ҳ����ֻת����������1 
	ADC_Init(ADC1, &ADC_InitStructure);//ADC��ʼ��
	
	//����ģʽͨ������ 
	ADC_RegularChannelConfig(ADC1,ADC_Channel_11,1, ADC_SampleTime_56Cycles);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_12,2, ADC_SampleTime_56Cycles);
	
	//ʹ��ADC��DMA 
	ADC_DMARequestAfterLastTransferCmd(ADC1,ENABLE); //Դ���ݱ仯ʱ����DMA����
	ADC_DMACmd(ADC1,ENABLE);//ʹ��ADC��DMA����
	 
	//ʹ��ADC1 
	ADC_Cmd(ADC1, ENABLE);//����ADת����	
	
	//����ADC1�����ת��
	ADC_SoftwareStartConv(ADC1);
}
//tips��static����������������ڶ�������Դ�ļ��ڣ����Բ���Ҫ��ͷ�ļ�������
static void OpenTimerForHc()        //�򿪶�ʱ��
{
    TIM_SetCounter(TIM7,0);//�������
    msHcCount = 0;
    TIM_Cmd(TIM7, ENABLE);  //ʹ��TIMx����
}
 
static void CloseTimerForHc()        //�رն�ʱ��
{
    TIM_Cmd(TIM7, DISABLE);  //ʹ��TIMx����
}
void HCSR04_init()
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;     //�������ڶ�ʱ�����õĽṹ��
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE); //ʹ��GPIOB��ʱ��
    //IO��ʼ��
    GPIO_InitStructure.GPIO_Pin =GPIO_Pin_6;       //���͵�ƽ����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;        //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//�ٶ�100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //�������

    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB,GPIO_Pin_6);
     
    GPIO_InitStructure.GPIO_Pin =   GPIO_Pin_7;     //���ص�ƽ����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//�ٶ�100MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);  
    GPIO_ResetBits(GPIOB,GPIO_Pin_7);    
     
    //��ʱ����ʼ�� ʹ�û�����ʱ��TIM6
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);   //ʹ�ܶ�ӦRCCʱ��
    //���ö�ʱ�������ṹ��
    TIM_DeInit(TIM7);
    TIM_TimeBaseStructure.TIM_Period = (1000-1); //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ         ������1000Ϊ1ms
    TIM_TimeBaseStructure.TIM_Prescaler =(84-1); //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  1M�ļ���Ƶ�� 1US����
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;//����Ƶ
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
    TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ         
        
    TIM_ClearFlag(TIM7, TIM_FLAG_Update);   //��������жϣ����һ���ж����������ж�
    TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE);    //�򿪶�ʱ�������ж�
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;             //ѡ�񴮿�1�ж�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //��ռʽ�ж����ȼ�����Ϊ1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;         //��Ӧʽ�ж����ȼ�����Ϊ1
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;        //ʹ���ж�
    NVIC_Init(&NVIC_InitStructure);
	
    TIM_Cmd(TIM7,DISABLE);     
}
//��ȡ��ʱ��ʱ��
u32 GetEchoTimer(void)
{
    u32 t = 0;
    t = msHcCount*1000;//�õ�MS
    t += TIM_GetCounter(TIM7);//�õ�US
    TIM7->CNT = 0;  //��TIM2�����Ĵ����ļ���ֵ����
    delay_ms(50);
    return t;
}
//һ�λ�ȡ������������� ���β��֮����Ҫ���һ��ʱ�䣬���ϻ����ź�
//Ϊ�����������Ӱ�죬ȡ������ݵ�ƽ��ֵ���м�Ȩ�˲���
float Hcsr04GetLength(void)
{
    u32 t = 0;
    int i = 0;
    float lengthTemp = 0;
    float sum = 0;
    while(i!=5)
    {
        TRIG_Send = 1;      //���Ϳڸߵ�ƽ���
        delay_us(20);
        TRIG_Send = 0;
        while(ECHO_Reci == 0);      //�ȴ����տڸߵ�ƽ���
        OpenTimerForHc();        //�򿪶�ʱ��
        i = i + 1;
        while(ECHO_Reci == 1);
        CloseTimerForHc();        //�رն�ʱ��
        t = GetEchoTimer();        //��ȡʱ��,�ֱ���Ϊ1US
        lengthTemp = ((float)t/58.0);//cm
        sum = lengthTemp + sum ;
    }
    lengthTemp = sum/5.0;
    return lengthTemp;
}
void TIM7_IRQHandler(void)   //TIM7�ж�
{
    if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)  //���TIM3�����жϷ������
    {
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update);  //���TIMx�����жϱ�־ 
        msHcCount++;
		voltage_count++;
		if(voltage_count >= 120000000)		//ÿ��2���ӣ���ʾ�����ٷֱȼ���1
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
