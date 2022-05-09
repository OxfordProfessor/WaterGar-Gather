#include "camera.h"
#include "usart.h"
#include "led.h"
#include "delay.h"
#include "stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "malloc.h"
#include "string.h"
#include "usart.h"
/*
  ���ļ���Ҫ�����������ݣ�
  1.������ݮ�ɵ�Uart����ͨ�ų�ʼ��
  2.������ݮ������Ľ����봦��Э�麯��
  3.�������˲�
*/

/*
  ��������
*/
u16 t;  
u16 len;
u16 times=0; //����Ҫ
int coords[2];
extern int AfterFliter[2];
extern KFP FilterX_Parameter, FilterY_Parameter; //�����������˲������ṹ��FilterX_Parameter��FilterY_Parameter
extern TaskHandle_t ControlServo_Handler;  //���������

float kalmanFilter(KFP *kfp,float input)   //�������˲�
{
     //Ԥ��Э����̣�kʱ��ϵͳ����Э���� = k-1ʱ�̵�ϵͳЭ���� + ��������Э����
     kfp->Now_P = kfp->LastP + kfp->Q;
     //���������淽�̣����������� = kʱ��ϵͳ����Э���� / ��kʱ��ϵͳ����Э���� + �۲�����Э���
     kfp->Kg = kfp->Now_P / (kfp->Now_P + kfp->R);
     //��������ֵ���̣�kʱ��״̬����������ֵ = ״̬������Ԥ��ֵ + ���������� * ������ֵ - ״̬������Ԥ��ֵ��
     kfp->out = kfp->out + kfp->Kg * (input -kfp->out);//��Ϊ��һ�ε�Ԥ��ֵ������һ�ε����ֵ
     //����Э�����: ���ε�ϵͳЭ����� kfp->LastP ����һ������׼����
     kfp->LastP = (1-kfp->Kg) * kfp->Now_P;
     return kfp->out;
}

void KFP_init(float LastP, float Now_P, float out, float Kg, float Q, float R, KFP* KFP)
{
	KFP->LastP = LastP;
	KFP->Now_P = Now_P;
	KFP->out = out;
	KFP->Kg = Kg;
	KFP->Q = Q;
	KFP->R = R;
}

void receivedata()
{
	u8 flag=0;
	char strX[4], strY[4];
	u8 cnt_x=0;
	u8 cnt_y=0;
	u8 *adress = NULL;
	u8 *date;
	if(USART_RX_STA&0x8000)		//�������������
	{	
		LED1=!LED1;//1��ָʾ�Ʊ��״̬
		len=USART_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
		date = mymalloc(SRAMIN,len+1);  //�����ڴ�
		sprintf((char*)date,"%s",USART_RX_BUF);
		adress = &date[0];	//ָ��adress�����ַ���ַ����0-len��һ��	
		//����Э��ȡ��������ַ���ʽ����strX��strY��
		while(t<len)				//��whileѭ������forѭ�����ٶȿ�3�������������������
		{
			t++;
			if(*adress>='0' && *adress<='9')
			{
				if(flag==1)
				{
					strX[cnt_x] = *adress;
					cnt_x++;
				}
				else
				{
					strY[cnt_y] = *adress;
					cnt_y++;
				}	
			}
			else
			{
				if(*adress=='#')
					flag = 1;
				if(*adress=='$')
					flag = 2;
			}
			adress++;	
		}
		//ת���ַ���Ϊ���ͣ����洢��ȫ�ֱ���coords��
		coords[0] = atoi(strX);
		coords[1] = atoi(strY);
		AfterFliter[0] = kalmanFilter(&FilterX_Parameter,coords[0]);		//X���˲�����
		AfterFliter[1] = kalmanFilter(&FilterX_Parameter,coords[1]);		//Y���˲�����
		//��־λ����
		USART_RX_STA=0;
		memset(USART_RX_BUF,0,USART_REC_LEN);			//���ڽ��ջ���������
		myfree(SRAMIN,date);						//�ͷ��ڴ�
	}
//		else
//		{
//			vTaskDelay(10);             	//��ʱ10ms��Ҳ����10��ʱ�ӽ���	; 
//		} 		 
}
