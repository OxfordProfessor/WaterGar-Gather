#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "sram.h"
#include "malloc.h"
#include "ILI93xx.h"
#include "led.h"
#include "timer.h"
#include "touch.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "ds18b20.h"
#include "task.h"
#include "limits.h"
#include "gui.h"
#include "GUI.h"
#include "GUIDemo.h"
#include "esp8266.h"
#include "serve.h"
#include "protocol.h"
#include "beep.h"
#include "pwm.h"
#include "sensor.h"
#include "camera.h"
#include "ff.h"
#include "exfuns.h"
#include "sensor.h"
#include <string.h>
#include <stdio.h>
#include "cJSON.h"
#include <stdlib.h>

//�������ȼ�
#define START_TASK_PRIO			1
//�����ջ��С	
#define START_STK_SIZE 			256  
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);

//TOUCH����
//�����������ȼ�
#define TOUCH_TASK_PRIO			2
//�����ջ��С
#define TOUCH_STK_SIZE			128
//������
TaskHandle_t TouchTask_Handler;
//touch����
void touch_task(void *pvParameters);

//LED0����
//�����������ȼ�
#define LED0_TASK_PRIO 			3
//�����ջ��С
#define LED0_STK_SIZE			128
//������
TaskHandle_t Led0Task_Handler;
//led0����
void led0_task(void *pvParameters);

//EMWINDEMO����
//�����������ȼ�
#define EMWINDEMO_TASK_PRIO		4
//�����ջ��С
#define EMWINDEMO_STK_SIZE		512
//������
TaskHandle_t EmwindemoTask_Handler;
//emwindemo_task����
void emwindemo_task(void *pvParameters);

//�ռ�����
//�����������ȼ�
#define COLLECT_TASK_PRIO		5
//�����ջ��С
#define COLLECT_STK_SIZE		128
//������
TaskHandle_t CollectTask_Handler;
//collect_task����
void collect_task(void *pvParameters);

//NET����
//�����������ȼ�
#define NET_TASK_PRIO		5
//�����ջ��С
#define NET_STK_SIZE		128
//������
TaskHandle_t NetTask_Handler;
//net_task����
void net_task(void *pvParameters);

//�ռ�����
//�����������ȼ�
#define MINORCONTROL_TASK_PRIO		5
//�����ջ��С
#define MINORCONTROL_STK_SIZE		128
//������
TaskHandle_t MinorControl_Handler;
//minorcontrol_task����
void minorcontrol_task(void *pvParameters);

//�ص���������
//�����������ȼ�
#define BACK_TASK_PRIO		6
//�����ջ��С
#define BACK_STK_SIZE		128
//������
TaskHandle_t Back_Handler;
//minorcontrol_task����
void back_task(void *pvParameters);

#define MSG_Q_NUM    1  		//������Ϣ���е����� 
QueueHandle_t Msg_Queue;   		//����ֵ��Ϣ���о��

typedef struct
{
	int time;
	int value;
}DIRECTION;

typedef struct
{
	int time;
	int value;
}COMEBACK;

const char *devSubTopic[] = {"/gvrxJiLWkq4/Stm32Internet/user/get"};	//��������
const char devPubTopic[] = "/gvrxJiLWkq4/Stm32Internet/user/update";	//��������

unsigned short timeCount = 0;	//���ͼ������
unsigned short timeCount_date = 0;	//���ͼ������
unsigned char *dataPtr = NULL;  //esp8266�����������ָ��
//����ȫ�ֱ���
KFP FilterX_Parameter, FilterY_Parameter; //�����������˲������ṹ��FilterX_Parameter��FilterY_Parameter
extern int coords[2];	//���ԭʼ���������
int AfterFliter[2];		//�������˲�����������x��y
FRESULT result;


extern char PUB_BUF1[256];
int main(void)
{
	delay_init(168);       	//��ʱ��ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 	//�жϷ�������
	usart1_init(115200);    //����1����������(���Ե��Խӿ�)
	usart3_init(115200);    //����3����������(Esp8266ͨ�Ŵ���)
	TFTLCD_Init();			//��ʼ��LCD
	TP_Init();				//��ʼ��������
	LED_Init();   			//LED��ʼ��
	DS18B20_Init();			//�¶ȴ�������ʼ��
	AllMoter_init();		//���е����ʼ��
	AllSensor_init();       //���д�������ʼ��
	FSMC_SRAM_Init(); 		//SRAM��ʼ��	
	KFP_init(0.02,0,0,0,0.001,0.543,&FilterX_Parameter);
	KFP_init(0.02,0,0,0,0.001,0.543,&FilterY_Parameter);
	exfuns_init();			//Ϊfatfs�ļ�ϵͳ�����ڴ�
	result = f_mount(fs[0],"0:",1);	//����SD��
	mem_init(SRAMIN); 		//�ڲ�RAM��ʼ��
	mem_init(SRAMEX); 		//�ⲿRAM��ʼ��
	mem_init(SRAMCCM);		//CCM��ʼ��
	UsartPrintf(USART_DEBUG, " Hardware init OK\r\n");
	ESP8266_Init();					//��ʼ��ESP8266
	while(OneNet_DevLink())			//����OneNET
		delay_ms(500);
	
	BEEP = 1;//������ʾ����ɹ�
	delay_ms(250);
	BEEP = 0;	
	OneNet_Subscribe(devSubTopic, 1);
	//������ʼ����
    xTaskCreate((TaskFunction_t )start_task,            //������
                (const char*    )"start_task",          //��������
                (uint16_t       )START_STK_SIZE,        //�����ջ��С
                (void*          )NULL,                  //���ݸ��������Ĳ���
                (UBaseType_t    )START_TASK_PRIO,       //�������ȼ�
                (TaskHandle_t*  )&StartTask_Handler);   //������                
    vTaskStartScheduler();          //�����������
}

//��ʼ����������
void start_task(void *pvParameters)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC,ENABLE);//����CRCʱ��
	WM_SetCreateFlags(WM_CF_MEMDEV);
	GUI_Init();  					//STemWin��ʼ��
	WM_MULTIBUF_Enable(1);  		//����STemWin�໺��,RGB�����ܻ��õ�
    taskENTER_CRITICAL();           //�����ٽ���
	//��������
	Msg_Queue=xQueueCreate(MSG_Q_NUM,sizeof(int));        //��������Msg_Queue
	//������������
    xTaskCreate((TaskFunction_t )touch_task,             
                (const char*    )"touch_task",           
                (uint16_t       )TOUCH_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )TOUCH_TASK_PRIO,        
                (TaskHandle_t*  )&TouchTask_Handler);   	
    //����LED0����
    xTaskCreate((TaskFunction_t )led0_task,             
                (const char*    )"led0_task",           
                (uint16_t       )LED0_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )LED0_TASK_PRIO,        
                (TaskHandle_t*  )&Led0Task_Handler);  
    //����EMWIN Demo����
    xTaskCreate((TaskFunction_t )emwindemo_task,             
                (const char*    )"emwindemo_task",           
                (uint16_t       )EMWINDEMO_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )EMWINDEMO_TASK_PRIO,        
                (TaskHandle_t*  )&EmwindemoTask_Handler);   
	//����Collect����
    xTaskCreate((TaskFunction_t )collect_task,             
                (const char*    )"collect_task",           
                (uint16_t       )COLLECT_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )COLLECT_TASK_PRIO,        
                (TaskHandle_t*  )&CollectTask_Handler);  
    //����Net����
    xTaskCreate((TaskFunction_t )net_task,             
                (const char*    )"net_task",           
                (uint16_t       )NET_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )NET_TASK_PRIO,        
                (TaskHandle_t*  )&NetTask_Handler);   			
    //����MinorControl����
    xTaskCreate((TaskFunction_t )minorcontrol_task,             
                (const char*    )"minorcontrol_task",           
                (uint16_t       )MINORCONTROL_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )MINORCONTROL_TASK_PRIO,        
                (TaskHandle_t*  )& MinorControl_Handler);   
    //����Back����
    xTaskCreate((TaskFunction_t )back_task,             
                (const char*    )"back_task",           
                (uint16_t       )BACK_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )BACK_TASK_PRIO,        
                (TaskHandle_t*  )& Back_Handler); 
	Back_speed_set(200);				//��������ʯ���������
	converyor_speed_set(200);			//�������ʹ�
	Collect_speed_set(200);				//����ǰ������ռ�
	vTaskSuspend(Back_Handler); 		//���ذ������񣬷Ǳ�Ҫ������
	vTaskSuspend(CollectTask_Handler);  //��ͣ�ռ����������񣩣��п�̨����ʱ�ſ�ʼִ�У���������
    vTaskDelete(StartTask_Handler); 	//ɾ����ʼ����
    taskEXIT_CRITICAL();            	//�˳��ٽ���
}

//EMWINDEMO����
void emwindemo_task(void *pvParameters)
{
	WM_MULTIBUF_Enable(1);
	while(1)
	{
		MainTask();
	}
}

//���������������
void touch_task(void *pvParameters)
{
	while(1)
	{
		GUI_TOUCH_Exec();	
		vTaskDelay(5);		//��ʱ5ms
	}
}

//LED0����
void led0_task(void *p_arg)
{
	while(1)
	{
		LED0 = !LED0;
		vTaskDelay(500);		//��ʱ500ms
	}
}
//�ռ�����
void collect_task(void *pvParameters)
{
	if(AfterFliter[0]<0)		//�������������
	{
		while(AfterFliter[0] > 20 || AfterFliter[0] < 0)  //����һ���������
		{
			turn_left(100);
		}
		
	}
	else if(AfterFliter[0]>0)	//�����������ұ�
	{
		while(AfterFliter[0] > 20 || AfterFliter[0] < 0)
		{
			turn_right(100);
		}
	}
	else	//δ��������,ǰ��
	{
		forward(200);
	}
	vTaskDelay(10); 
}
//���������ƺ�������������������ݡ����ջ��������ݿ�������
void net_task(void *pvParameters) 
{
	int temp,tds,storage,voltage;
	float ph;
	int dir;
	int com;
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};								//Э���
	
	char *req_payload = NULL;
	char *cmdid_topic = NULL;
	
	unsigned short topic_len = 0;
	unsigned short req_len = 0;
	
	unsigned char type = 0;
	unsigned char qos = 0;
	static unsigned short pkt_id = 0;
	unsigned char *cmd = NULL;
	short result = 0;

	char *dataPtr = NULL;
	char numBuf[10];
	int num = 0;
	BaseType_t err;
	cJSON *json , *json_value;
	
	while(1)
	{
		timeCount++;
		timeCount_date++;
		if(timeCount_date % 500 == 0)   //1000ms / 25 = 10 һ��ִ��һ�Σ������¶ȴ��������ݣ�
		{
			//�����¶�����
			temp = (int)(DS18B20_Get_Temp()*0.1);
			tds = (int)(current_tds());
			ph = current_ph();
			storage = margin_storage_percent();
			voltage = margin_voltage_percent();
			//�򻺳���PUB_BUF1��д���¶�����
			sprintf(PUB_BUF1,"{\"params\":{\"Temp\":%d,\"ph\":%lf,\"tds\":%d,\"storage\":%d,\"voltage\":%d},\"method\":\"thing.event.property.post\"}", 
											temp,ph,tds,storage,voltage);    //��������
			//�����������������PUB_BUF1��Ϣ��������ʪ�ȡ��¶�ֵ
			OneNet_Publish("/sys/gvrxJiLWkq4/Stm32Internet/thing/event/property/post", PUB_BUF1); 
			timeCount_date = 0;
			cmd = ESP8266_GetIPD(3);
			type = MQTT_UnPacketRecv(cmd);
			switch(type)
			{
				case MQTT_PKT_CMD:															//�����·�
					result = MQTT_UnPacketCmd(cmd, &cmdid_topic, &req_payload, &req_len);	//���topic����Ϣ��
					if(result == 0)
					{
						UsartPrintf(USART_DEBUG, "cmdid: %s, req: %s, req_len: %d\r\n", cmdid_topic, req_payload, req_len);
						MQTT_DeleteBuffer(&mqttPacket);									//ɾ��
					}
				break;
				
				case MQTT_PKT_PUBLISH:														//���յ�Publish��Ϣ
					result = MQTT_UnPacketPublish(cmd, &cmdid_topic, &topic_len, &req_payload, &req_len, &qos, &pkt_id);
					if(result == 0)
					{
						xTaskNotifyGive(MinorControl_Handler);	//����Ϊ��������������֪ͨ�����������״̬
						UsartPrintf(USART_DEBUG, "topic: %s, topic_len: %d, payload: %s, payload_len: %d\r\n",
																	cmdid_topic, topic_len, req_payload, req_len);
						// �����ݰ�req_payload����JSON��ʽ����
						json = cJSON_Parse(req_payload);
						if (!json)UsartPrintf(USART_DEBUG,"Error before: [%s]\n",cJSON_GetErrorPtr());
						else
						{
							json_value = cJSON_GetObjectItem(json , "wechat");
//							UsartPrintf(USART_DEBUG,"json_value: [%s]\n",json_value->string);
							UsartPrintf(USART_DEBUG,"json_value: [%s]\n",json_value->valuestring);
							cJSON *root = cJSON_Parse(req_payload);
							cJSON *object = cJSON_GetObjectItem(root,"direction");
							cJSON *item;
							DIRECTION direction;
							item=cJSON_GetObjectItem(object,"value");
							direction.value = item->valueint;		//����dirction.value��Ϣ
							item=cJSON_GetObjectItem(object,"time");
							direction.time = item->valueint;		//����dirction.time��Ϣ	
							dir = direction.value;
							cJSON *object_comeback = cJSON_GetObjectItem(root,"comeback");
							cJSON *item_comeback;
							COMEBACK comeback;
							item_comeback=cJSON_GetObjectItem(object_comeback,"value");
							comeback.value = item_comeback->valueint;		//����comeback.value��Ϣ
							item_comeback=cJSON_GetObjectItem(object_comeback,"time");
							comeback.time = item_comeback->valueint;		//����comeback.time��Ϣ	
							com = comeback.value;
							if(com == 1)		//һ���ӵ������źţ����������������ȼ���ߣ����ɱ�����������
							{
								vTaskResume(Back_Handler);	 //�ָ�����
							}
							if(Msg_Queue!=NULL)
							{
								err = xQueueSend(Msg_Queue,&dir,10);
								if(err == errQUEUE_FULL)
								{
									UsartPrintf(USART_DEBUG,"��������");
								}
							}
						}
						cJSON_Delete(json);
					}
				break;
			
				case MQTT_PKT_PUBACK:					//����Publish��Ϣ��ƽ̨�ظ���Ack
					if(MQTT_UnPacketPublishAck(cmd) == 0)
						UsartPrintf(USART_DEBUG, "Tips:	MQTT Publish Send OK\r\n");
				break;
					
				case MQTT_PKT_PUBREC:						//����Publish��Ϣ��ƽ̨�ظ���Rec���豸��ظ�Rel��Ϣ
					if(MQTT_UnPacketPublishRec(cmd) == 0)
					{
						UsartPrintf(USART_DEBUG, "Tips:	Rev PublishRec\r\n");
						if(MQTT_PacketPublishRel(MQTT_PUBLISH_ID, &mqttPacket) == 0)
						{
							UsartPrintf(USART_DEBUG, "Tips:	Send PublishRel\r\n");
							ESP8266_SendData(mqttPacket._data, mqttPacket._len);
							MQTT_DeleteBuffer(&mqttPacket);
						}
					}
				break;
					
				case MQTT_PKT_PUBREL:			//�յ�Publish��Ϣ���豸�ظ�Rec��ƽ̨�ظ���Rel���豸���ٻظ�Comp
					if(MQTT_UnPacketPublishRel(cmd, pkt_id) == 0)
					{
						UsartPrintf(USART_DEBUG, "Tips:	Rev PublishRel\r\n");
						if(MQTT_PacketPublishComp(MQTT_PUBLISH_ID, &mqttPacket) == 0)
						{
							UsartPrintf(USART_DEBUG, "Tips:	Send PublishComp\r\n");
							ESP8266_SendData(mqttPacket._data, mqttPacket._len);
							MQTT_DeleteBuffer(&mqttPacket);
						}
					}
				break;
					
				case MQTT_PKT_PUBCOMP:			//����Publish��Ϣ��ƽ̨����Rec���豸�ظ�Rel��ƽ̨�ٷ��ص�Comp
					if(MQTT_UnPacketPublishComp(cmd) == 0)
					{
						UsartPrintf(USART_DEBUG, "Tips:	Rev PublishComp\r\n");
					}
				break;
			
				case MQTT_PKT_SUBACK:				//����Subscribe��Ϣ��Ack
					if(MQTT_UnPacketSubscribe(cmd) == 0)
						UsartPrintf(USART_DEBUG, "Tips:	MQTT Subscribe OK\r\n");
					else
						UsartPrintf(USART_DEBUG, "Tips:	MQTT Subscribe Err\r\n");
				break;
			
				case MQTT_PKT_UNSUBACK:														//����UnSubscribe��Ϣ��Ack
		
					if(MQTT_UnPacketUnSubscribe(cmd) == 0)
						UsartPrintf(USART_DEBUG, "Tips:	MQTT UnSubscribe OK\r\n");
					else
						UsartPrintf(USART_DEBUG, "Tips:	MQTT UnSubscribe Err\r\n");
				break;

				default:
				result = -1;
				break;
			}
			ESP8266_Clear();									//��ջ���
			if(result == -1)
				return;
			dataPtr = strchr(req_payload, '}');					//����'}'
			if(dataPtr != NULL && result != -1)					//����ҵ���
			{
				dataPtr++;
				while(*dataPtr >= '0' && *dataPtr <= '9')		//�ж��Ƿ����·��������������
				{
					numBuf[num++] = *dataPtr++;
				}
				num = atoi((const char *)numBuf);				//תΪ��ֵ��ʽ
			}
			if(type == MQTT_PKT_CMD || type == MQTT_PKT_PUBLISH)
			{
				MQTT_FreeBuffer(cmdid_topic);
				MQTT_FreeBuffer(req_payload);
			}
		}
		if(++timeCount >= 5000)									//���ͼ��
		{
			OneNet_Ping();		//���������ܻ������ж�
			timeCount=0; 
		}
		else
		{
			vTaskDelay(50);
		}
	}
}
//��Ϊ������������
void minorcontrol_task(void *pvParameters)
{
	u32 NotifyValue;
	int dir;
	while(1)
	{
		NotifyValue = ulTaskNotifyTake(pdTRUE,portMAX_DELAY);	//���߳�ֻ���п�������ʱ�Ŵ���������ʱ�䲻����
		if(NotifyValue == 1)  //˵����������Ϊ�������ƣ���ʱռ���߳�
		{
			vTaskSuspend(CollectTask_Handler); //�����ռ����񣬵Ƚ������ٻָ�
			if(Msg_Queue!=NULL)
			{
				xQueueReceive(Msg_Queue,&dir,portMAX_DELAY);
				switch(dir)
				{
					case 0:
						forward(200);
					break;
					case 1:
						turn_left(200);
					break;
					case 2:
						turn_left(250);
					break;
					case 3:
						turn_left(300);
					break;
					case 4:
						turn_left(350);
					break;
					case 5:
						turn_right(200);
					break;	
					case 6:
						turn_right(200);
					break;
					case 7:
						turn_right(200);
					break;					
				}
			}
			vTaskResume(CollectTask_Handler);	 //�ָ�����
		}
		else		//һֱ����
		{
			vTaskDelay(10); 
		}
	}
}
//�ص���������
void back_task(void *pvParameters)
{
	turn_left(300);
	delay_ms(2000);
	while(1)
	{
		forward(200);
	}
}
