#include "pwm.h"

/*
  ���ļ���Ҫ�����������ݣ�
  1.����ˮ����ˢ���������ʼ��(PWM)
  2.һ�����������(PWM)
  3.һ�����ʹ���������PWM��
  4.������ǰ�����ռ���������PWM��
*/

/*
  �˿ڶ���
#define LeftDrive_Moter_P   	TIM1CH1   	PE9
#define LeftDrive_Moter_N		TIM1CH4   	PE14
#define RightDrive_Moter_P		TIM2CH1		PA0
#define RightDrive_Moter_N		TIM2CH2		PA1
#define Back_Moter_P			TIM3CH1		PA6
#define Back_Moter_N			TIM3CH2		PA7
#define Converyor_Moter_P		TIM4CH3		PD12
#define Converyor_Moter_N		TIM4CH4		PD13
#define LeftCollect_Moter_P		TIM8CH1		PC6
#define LeftCollect_Moter_N		TIM8CH2		PC7
#define RightCollect_Moter_P	TIM9CH1		PE5
#define RightCollect_Moter_N	TIM9CH2		PE6
*/

/*
  ��������
*/

void TIM1_PWM_Init(u16 per,u16 psc)        //����ƽ����PWM��ʼ��
{ 
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource9,GPIO_AF_TIM1); //GPIOA8����Ϊ��ʱ��1CH1
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource14,GPIO_AF_TIM1); //GPIOA8����Ϊ��ʱ��1CH4
	                                                                     	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_14;      //need to reset the pin of the righr motor,this is the four channels of TIM1.
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//�ٶ�100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        //����
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	TIM_TimeBaseStructure.TIM_Period = per; 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); 

	//����ͨ��1���˴��ṹ�����ȫ�����ã������������PWM����������
	TIM_OCInitStructure.TIM_Pulse = 0; 		//��װ�벶��ȽϼĴ���������ֵ
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;  //TIM1�����ȵ���ģʽ1
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//ʹ������Ƚ�״̬
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;//ʹ������Ƚ�N״̬
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;//TIM1 ����Ƚϼ��Ը�
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;//TIM1 ����Ƚ�N���Ը�
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;//�� MOE=0 ���� TIM1 ����ȽϿ���״̬
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;//�� MOE=0 ���� TIM1 ����Ƚ� N ����״̬
	TIM_OC1Init(TIM1, &TIM_OCInitStructure); //����Ƚ�ͨ��1��ʼ��  

	//����ͨ��2���˴��ṹ�����ȫ�����ã������������PWM����������
	TIM_OCInitStructure.TIM_Pulse = 0; 		//��װ�벶��ȽϼĴ���������ֵ
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;  //TIM1�����ȵ���ģʽ1
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//ʹ������Ƚ�״̬
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;//ʹ������Ƚ�N״̬
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;//TIM1 ����Ƚϼ��Ը�
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;//TIM1 ����Ƚ�N���Ը�
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;//�� MOE=0 ���� TIM1 ����ȽϿ���״̬
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;//�� MOE=0 ���� TIM1 ����Ƚ� N ����״̬
	TIM_OC4Init(TIM1, &TIM_OCInitStructure); //����Ƚ�ͨ��4��ʼ��
	
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);//ʹ��TIMx�� CCR1 �ϵ�Ԥװ 
	TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);//ʹ��TIMx�� CCR4 �ϵ�Ԥװ
	TIM_ARRPreloadConfig(TIM1, ENABLE);//ʹ��Ԥװ�ؼĴ���
	
	TIM_CtrlPWMOutputs(TIM1,ENABLE);
	TIM_Cmd(TIM1, ENABLE);  //ʹ�ܶ�ʱ��	
}
void TIM2_PWM_Init(u16 per,u16 psc)        //�Ҳ��ƽ����PWM��ʼ��
{ 
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource0,GPIO_AF_TIM1); //GPIOA8����Ϊ��ʱ��1CH1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource1,GPIO_AF_TIM1); //GPIOA8����Ϊ��ʱ��1CH4
	                                                                     	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;      //need to reset the pin of the righr motor,this is the four channels of TIM1.
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//�ٶ�100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        //����
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	TIM_TimeBaseStructure.TIM_Period = per; 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); 

	//����ͨ��1���˴��ṹ�����ȫ�����ã������������PWM����������
	TIM_OCInitStructure.TIM_Pulse = 0; 		//��װ�벶��ȽϼĴ���������ֵ
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;  //TIM1�����ȵ���ģʽ1
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//ʹ������Ƚ�״̬
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;//ʹ������Ƚ�N״̬
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;//TIM1 ����Ƚϼ��Ը�
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;//TIM1 ����Ƚ�N���Ը�
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;//�� MOE=0 ���� TIM1 ����ȽϿ���״̬
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;//�� MOE=0 ���� TIM1 ����Ƚ� N ����״̬
	TIM_OC1Init(TIM2, &TIM_OCInitStructure); //����Ƚ�ͨ��1��ʼ��  

	//����ͨ��2���˴��ṹ�����ȫ�����ã������������PWM����������
	TIM_OCInitStructure.TIM_Pulse = 0; 		//��װ�벶��ȽϼĴ���������ֵ
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;  //TIM1�����ȵ���ģʽ1
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//ʹ������Ƚ�״̬
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;//ʹ������Ƚ�N״̬
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;//TIM1 ����Ƚϼ��Ը�
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;//TIM1 ����Ƚ�N���Ը�
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;//�� MOE=0 ���� TIM1 ����ȽϿ���״̬
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;//�� MOE=0 ���� TIM1 ����Ƚ� N ����״̬
	TIM_OC2Init(TIM2, &TIM_OCInitStructure); //����Ƚ�ͨ��4��ʼ��
	
	TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);//ʹ��TIMx�� CCR1 �ϵ�Ԥװ 
	TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);//ʹ��TIMx�� CCR4 �ϵ�Ԥװ
	TIM_ARRPreloadConfig(TIM2, ENABLE);//ʹ��Ԥװ�ؼĴ���
	
	TIM_CtrlPWMOutputs(TIM2,ENABLE);
	TIM_Cmd(TIM2, ENABLE);  //ʹ�ܶ�ʱ��	
}
void TIM3_PWM_Init(u16 per,u16 psc)        //���ʯ���������PWM��ʼ��
{ 
	//�˲������ֶ��޸�IO������
	
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  	//TIM3ʱ��ʹ��    	
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_TIM3); //GPIOA6����Ϊ��ʱ��3
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_TIM3); //GPIOA7����Ϊ��ʱ��3
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//�ٶ�100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        //����
	GPIO_Init(GPIOA,&GPIO_InitStructure);              //��ʼ��
	  
	TIM_TimeBaseStructure.TIM_Prescaler=psc;  //��ʱ����Ƶ
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseStructure.TIM_Period=per;   //�Զ���װ��ֵ
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);//��ʼ����ʱ��3
	
	//��ʼ��TIM1 Channel1 PWMģʽ	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //�������:TIM����Ƚϼ��Ե�
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);  //����Tָ���Ĳ�����ʼ������TIM1 4OC1
	
	//��ʼ��TIM1 Channel2 PWMģʽ	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //�������:TIM����Ƚϼ��Ե�
	TIM_OC2Init(TIM3, &TIM_OCInitStructure);  //����Tָ���Ĳ�����ʼ������TIM1 4OC1

	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);  //ʹ��TIM4��CCR1�ϵ�Ԥװ�ؼĴ���
	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);  //ʹ��TIM4��CCR2�ϵ�Ԥװ�ؼĴ���
 
    TIM_ARRPreloadConfig(TIM3,ENABLE);//ARPEʹ�� 
	
	TIM_Cmd(TIM3, ENABLE);  //ʹ��TIM3
 
}
void TIM4_PWM_Init(u16 per,u16 psc)        //���ʹ�������PWM��ʼ��
{ 
	//�˲������ֶ��޸�IO������
	
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);  	//TIM3ʱ��ʹ��    	
	
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource12,GPIO_AF_TIM4); //GPIOA6����Ϊ��ʱ��3
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource13,GPIO_AF_TIM4); //GPIOA7����Ϊ��ʱ��3
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//�ٶ�100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        //����
	GPIO_Init(GPIOD,&GPIO_InitStructure);              //��ʼ��
	  
	TIM_TimeBaseStructure.TIM_Prescaler=psc;  //��ʱ����Ƶ
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseStructure.TIM_Period=per;   //�Զ���װ��ֵ
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseStructure);//��ʼ����ʱ��3
	
	//��ʼ��TIM1 Channel1 PWMģʽ	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //�������:TIM����Ƚϼ��Ե�
	TIM_OC1Init(TIM4, &TIM_OCInitStructure);  //����Tָ���Ĳ�����ʼ������TIM1 4OC1
	
	//��ʼ��TIM1 Channel2 PWMģʽ	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //�������:TIM����Ƚϼ��Ե�
	TIM_OC2Init(TIM4, &TIM_OCInitStructure);  //����Tָ���Ĳ�����ʼ������TIM1 4OC1

	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);  //ʹ��TIM4��CCR1�ϵ�Ԥװ�ؼĴ���
	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);  //ʹ��TIM4��CCR2�ϵ�Ԥװ�ؼĴ���
 
    TIM_ARRPreloadConfig(TIM4,ENABLE);//ARPEʹ�� 
	
	TIM_Cmd(TIM4, ENABLE);  //ʹ��TIM3
 
}
void TIM8_PWM_Init(u16 per,u16 psc)        //��������ռ����PWM����
{ 
	//�˲������ֶ��޸�IO������
	
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB2Periph_TIM8,ENABLE);  	//TIM3ʱ��ʹ��    	
	
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource6,GPIO_AF_TIM8); //GPIOA6����Ϊ��ʱ��3
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource7,GPIO_AF_TIM8); //GPIOA7����Ϊ��ʱ��3
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//�ٶ�100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        //����
	GPIO_Init(GPIOC,&GPIO_InitStructure);              //��ʼ��
	  
	TIM_TimeBaseStructure.TIM_Prescaler=psc;  //��ʱ����Ƶ
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseStructure.TIM_Period=per;   //�Զ���װ��ֵ
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM8,&TIM_TimeBaseStructure);//��ʼ����ʱ��3
	
	//��ʼ��TIM1 Channel1 PWMģʽ	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //�������:TIM����Ƚϼ��Ե�
	TIM_OC1Init(TIM8, &TIM_OCInitStructure);  //����Tָ���Ĳ�����ʼ������TIM1 4OC1
	
	//��ʼ��TIM1 Channel2 PWMģʽ	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //�������:TIM����Ƚϼ��Ե�
	TIM_OC2Init(TIM8, &TIM_OCInitStructure);  //����Tָ���Ĳ�����ʼ������TIM1 4OC1

	TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Enable);  //ʹ��TIM4��CCR1�ϵ�Ԥװ�ؼĴ���
	TIM_OC2PreloadConfig(TIM8, TIM_OCPreload_Enable);  //ʹ��TIM4��CCR2�ϵ�Ԥװ�ؼĴ���
 
    TIM_ARRPreloadConfig(TIM8,ENABLE);//ARPEʹ�� 
	
	TIM_Cmd(TIM8, ENABLE);  //ʹ��TIM3
 
}
void TIM9_PWM_Init(u16 per,u16 psc)        //��������ռ����PWM����
{ 
	//�˲������ֶ��޸�IO������
	
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB2Periph_TIM9,ENABLE);  	//TIM3ʱ��ʹ��    	
	
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource5,GPIO_AF_TIM9); //GPIOA6����Ϊ��ʱ��3
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource6,GPIO_AF_TIM9); //GPIOA7����Ϊ��ʱ��3
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//�ٶ�100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        //����
	GPIO_Init(GPIOE,&GPIO_InitStructure);              //��ʼ��
	  
	TIM_TimeBaseStructure.TIM_Prescaler=psc;  //��ʱ����Ƶ
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseStructure.TIM_Period=per;   //�Զ���װ��ֵ
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM9,&TIM_TimeBaseStructure);//��ʼ����ʱ��3
	
	//��ʼ��TIM1 Channel1 PWMģʽ	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //�������:TIM����Ƚϼ��Ե�
	TIM_OC1Init(TIM9, &TIM_OCInitStructure);  //����Tָ���Ĳ�����ʼ������TIM1 4OC1
	
	//��ʼ��TIM1 Channel2 PWMģʽ	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //�������:TIM����Ƚϼ��Ե�
	TIM_OC2Init(TIM9, &TIM_OCInitStructure);  //����Tָ���Ĳ�����ʼ������TIM1 4OC1

	TIM_OC1PreloadConfig(TIM9, TIM_OCPreload_Enable);  //ʹ��TIM4��CCR1�ϵ�Ԥװ�ؼĴ���
	TIM_OC2PreloadConfig(TIM9, TIM_OCPreload_Enable);  //ʹ��TIM4��CCR2�ϵ�Ԥװ�ؼĴ���
 
    TIM_ARRPreloadConfig(TIM9,ENABLE);//ARPEʹ�� 
	
	TIM_Cmd(TIM9, ENABLE);  //ʹ��TIM3
 
}
void AllMoter_init()  //��ʼ������PWM
{
	TIM1_PWM_Init(1000-1,84-1);
	TIM2_PWM_Init(1000-1,84-1);
	TIM3_PWM_Init(1000-1,84-1);
	TIM4_PWM_Init(1000-1,84-1);
	TIM8_PWM_Init(1000-1,84-1);
	TIM9_PWM_Init(1000-1,84-1);
}
void forward(int speed)    //ǰ�������ֻ����˳ʱ��ת
{
	if(speed<0)speed = -speed;
	if(speed>800) speed = 800;
	TIM_SetCompare1(TIM1,speed);	//�޸ıȽ�ֵ���޸�ռ�ձ�
	TIM_SetCompare4(TIM1,0);	//�޸ıȽ�ֵ���޸�ռ�ձ�
	TIM_SetCompare1(TIM2,speed);	//�޸ıȽ�ֵ���޸�ռ�ձ�
	TIM_SetCompare2(TIM2,0);	//�޸ıȽ�ֵ���޸�ռ�ձ�
}
void turn_left(int speed)   //��ת
{
	if(speed<0)speed = -speed;
	if(speed>600) speed = 600;
	TIM_SetCompare1(TIM1,speed);	//�޸ıȽ�ֵ���޸�ռ�ձ�
	TIM_SetCompare4(TIM1,0);	//�޸ıȽ�ֵ���޸�ռ�ձ�
	TIM_SetCompare1(TIM2,200+speed);	//�޸ıȽ�ֵ���޸�ռ�ձ�
	TIM_SetCompare2(TIM2,0);	//�޸ıȽ�ֵ���޸�ռ�ձ�
}
void turn_right(int speed)   //��ת
{
	if(speed<0)speed = -speed;
	if(speed>600) speed = 600;
	TIM_SetCompare1(TIM1,200+speed);	//�޸ıȽ�ֵ���޸�ռ�ձ�
	TIM_SetCompare4(TIM1,0);	//�޸ıȽ�ֵ���޸�ռ�ձ�
	TIM_SetCompare1(TIM2,speed);	//�޸ıȽ�ֵ���޸�ռ�ձ�
	TIM_SetCompare2(TIM2,0);	//�޸ıȽ�ֵ���޸�ռ�ձ�
}
void converyor_speed_set(int speed) //���ʹ��ٶ�����,�������
{
	if(speed<0)speed = -speed;
	if(speed>800) speed = 800;
	TIM_SetCompare1(TIM4,speed);	//�޸ıȽ�ֵ���޸�ռ�ձ�
	TIM_SetCompare2(TIM4,0);	//�޸ıȽ�ֵ���޸�ռ�ձ�
}
void Back_speed_set(int speed)   //����ռ�ʯ�͹����ٶ�����
{
	if(speed<0)speed = -speed;
	if(speed>800) speed = 800;
	TIM_SetCompare1(TIM3,speed);	//�޸ıȽ�ֵ���޸�ռ�ձ�
	TIM_SetCompare2(TIM3,0);	//�޸ıȽ�ֵ���޸�ռ�ձ�
}
void Collect_speed_set(int speed)   //ǰ���ռ�����ת���ٶ�����
{
	if(speed<0)speed = -speed;
	if(speed>800) speed = 800;
	TIM_SetCompare1(TIM8,speed);	//�޸ıȽ�ֵ���޸�ռ�ձ�
	TIM_SetCompare2(TIM8,0);	//�޸ıȽ�ֵ���޸�ռ�ձ�
	TIM_SetCompare1(TIM9,speed);	//�޸ıȽ�ֵ���޸�ռ�ձ�
	TIM_SetCompare2(TIM9,0);	//�޸ıȽ�ֵ���޸�ռ�ձ�
}