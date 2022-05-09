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

//任务优先级
#define START_TASK_PRIO			1
//任务堆栈大小	
#define START_STK_SIZE 			256  
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void *pvParameters);

//TOUCH任务
//设置任务优先级
#define TOUCH_TASK_PRIO			2
//任务堆栈大小
#define TOUCH_STK_SIZE			128
//任务句柄
TaskHandle_t TouchTask_Handler;
//touch任务
void touch_task(void *pvParameters);

//LED0任务
//设置任务优先级
#define LED0_TASK_PRIO 			3
//任务堆栈大小
#define LED0_STK_SIZE			128
//任务句柄
TaskHandle_t Led0Task_Handler;
//led0任务
void led0_task(void *pvParameters);

//EMWINDEMO任务
//设置任务优先级
#define EMWINDEMO_TASK_PRIO		4
//任务堆栈大小
#define EMWINDEMO_STK_SIZE		512
//任务句柄
TaskHandle_t EmwindemoTask_Handler;
//emwindemo_task任务
void emwindemo_task(void *pvParameters);

//收集任务
//设置任务优先级
#define COLLECT_TASK_PRIO		5
//任务堆栈大小
#define COLLECT_STK_SIZE		128
//任务句柄
TaskHandle_t CollectTask_Handler;
//collect_task任务
void collect_task(void *pvParameters);

//NET任务
//设置任务优先级
#define NET_TASK_PRIO		5
//任务堆栈大小
#define NET_STK_SIZE		128
//任务句柄
TaskHandle_t NetTask_Handler;
//net_task任务
void net_task(void *pvParameters);

//收集任务
//设置任务优先级
#define MINORCONTROL_TASK_PRIO		5
//任务堆栈大小
#define MINORCONTROL_STK_SIZE		128
//任务句柄
TaskHandle_t MinorControl_Handler;
//minorcontrol_task任务
void minorcontrol_task(void *pvParameters);

//回到岸边任务
//设置任务优先级
#define BACK_TASK_PRIO		6
//任务堆栈大小
#define BACK_STK_SIZE		128
//任务句柄
TaskHandle_t Back_Handler;
//minorcontrol_task任务
void back_task(void *pvParameters);

#define MSG_Q_NUM    1  		//传递消息队列的数量 
QueueHandle_t Msg_Queue;   		//按键值消息队列句柄

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

const char *devSubTopic[] = {"/gvrxJiLWkq4/Stm32Internet/user/get"};	//订阅主题
const char devPubTopic[] = "/gvrxJiLWkq4/Stm32Internet/user/update";	//发布主题

unsigned short timeCount = 0;	//发送间隔变量
unsigned short timeCount_date = 0;	//发送间隔变量
unsigned char *dataPtr = NULL;  //esp8266正常运作检查指针
//定义全局变量
KFP FilterX_Parameter, FilterY_Parameter; //两个卡尔曼滤波参数结构体FilterX_Parameter与FilterY_Parameter
extern int coords[2];	//存放原始坐标的数组
int AfterFliter[2];		//卡尔曼滤波后物体坐标x与y
FRESULT result;


extern char PUB_BUF1[256];
int main(void)
{
	delay_init(168);       	//延时初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 	//中断分组配置
	usart1_init(115200);    //串口1波特率设置(电脑调试接口)
	usart3_init(115200);    //串口3波特率设置(Esp8266通信串口)
	TFTLCD_Init();			//初始化LCD
	TP_Init();				//初始化触摸屏
	LED_Init();   			//LED初始化
	DS18B20_Init();			//温度传感器初始化
	AllMoter_init();		//所有电机初始化
	AllSensor_init();       //所有传感器初始化
	FSMC_SRAM_Init(); 		//SRAM初始化	
	KFP_init(0.02,0,0,0,0.001,0.543,&FilterX_Parameter);
	KFP_init(0.02,0,0,0,0.001,0.543,&FilterY_Parameter);
	exfuns_init();			//为fatfs文件系统分配内存
	result = f_mount(fs[0],"0:",1);	//挂载SD卡
	mem_init(SRAMIN); 		//内部RAM初始化
	mem_init(SRAMEX); 		//外部RAM初始化
	mem_init(SRAMCCM);		//CCM初始化
	UsartPrintf(USART_DEBUG, " Hardware init OK\r\n");
	ESP8266_Init();					//初始化ESP8266
	while(OneNet_DevLink())			//接入OneNET
		delay_ms(500);
	
	BEEP = 1;//鸣叫提示接入成功
	delay_ms(250);
	BEEP = 0;	
	OneNet_Subscribe(devSubTopic, 1);
	//创建开始任务
    xTaskCreate((TaskFunction_t )start_task,            //任务函数
                (const char*    )"start_task",          //任务名称
                (uint16_t       )START_STK_SIZE,        //任务堆栈大小
                (void*          )NULL,                  //传递给任务函数的参数
                (UBaseType_t    )START_TASK_PRIO,       //任务优先级
                (TaskHandle_t*  )&StartTask_Handler);   //任务句柄                
    vTaskStartScheduler();          //开启任务调度
}

//开始任务任务函数
void start_task(void *pvParameters)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC,ENABLE);//开启CRC时钟
	WM_SetCreateFlags(WM_CF_MEMDEV);
	GUI_Init();  					//STemWin初始化
	WM_MULTIBUF_Enable(1);  		//开启STemWin多缓冲,RGB屏可能会用到
    taskENTER_CRITICAL();           //进入临界区
	//创建队列
	Msg_Queue=xQueueCreate(MSG_Q_NUM,sizeof(int));        //创建队列Msg_Queue
	//创建触摸任务
    xTaskCreate((TaskFunction_t )touch_task,             
                (const char*    )"touch_task",           
                (uint16_t       )TOUCH_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )TOUCH_TASK_PRIO,        
                (TaskHandle_t*  )&TouchTask_Handler);   	
    //创建LED0任务
    xTaskCreate((TaskFunction_t )led0_task,             
                (const char*    )"led0_task",           
                (uint16_t       )LED0_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )LED0_TASK_PRIO,        
                (TaskHandle_t*  )&Led0Task_Handler);  
    //创建EMWIN Demo任务
    xTaskCreate((TaskFunction_t )emwindemo_task,             
                (const char*    )"emwindemo_task",           
                (uint16_t       )EMWINDEMO_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )EMWINDEMO_TASK_PRIO,        
                (TaskHandle_t*  )&EmwindemoTask_Handler);   
	//创建Collect任务
    xTaskCreate((TaskFunction_t )collect_task,             
                (const char*    )"collect_task",           
                (uint16_t       )COLLECT_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )COLLECT_TASK_PRIO,        
                (TaskHandle_t*  )&CollectTask_Handler);  
    //创建Net任务
    xTaskCreate((TaskFunction_t )net_task,             
                (const char*    )"net_task",           
                (uint16_t       )NET_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )NET_TASK_PRIO,        
                (TaskHandle_t*  )&NetTask_Handler);   			
    //创建MinorControl任务
    xTaskCreate((TaskFunction_t )minorcontrol_task,             
                (const char*    )"minorcontrol_task",           
                (uint16_t       )MINORCONTROL_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )MINORCONTROL_TASK_PRIO,        
                (TaskHandle_t*  )& MinorControl_Handler);   
    //创建Back任务
    xTaskCreate((TaskFunction_t )back_task,             
                (const char*    )"back_task",           
                (uint16_t       )BACK_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )BACK_TASK_PRIO,        
                (TaskHandle_t*  )& Back_Handler); 
	Back_speed_set(200);				//启动后置石油吸附电机
	converyor_speed_set(200);			//启动传送带
	Collect_speed_set(200);				//启动前向滚轮收集
	vTaskSuspend(Back_Handler); 		//返回岸边任务，非必要不启动
	vTaskSuspend(CollectTask_Handler);  //暂停收集任务（主任务），中控台启动时才开始执行，否则不启动
    vTaskDelete(StartTask_Handler); 	//删除开始任务
    taskEXIT_CRITICAL();            	//退出临界区
}

//EMWINDEMO任务
void emwindemo_task(void *pvParameters)
{
	WM_MULTIBUF_Enable(1);
	while(1)
	{
		MainTask();
	}
}

//触摸任务的任务函数
void touch_task(void *pvParameters)
{
	while(1)
	{
		GUI_TOUCH_Exec();	
		vTaskDelay(5);		//延时5ms
	}
}

//LED0任务
void led0_task(void *p_arg)
{
	while(1)
	{
		LED0 = !LED0;
		vTaskDelay(500);		//延时500ms
	}
}
//收集任务
void collect_task(void *pvParameters)
{
	if(AfterFliter[0]<0)		//发现物体在左边
	{
		while(AfterFliter[0] > 20 || AfterFliter[0] < 0)  //给定一定幅度误差
		{
			turn_left(100);
		}
		
	}
	else if(AfterFliter[0]>0)	//发现物体在右边
	{
		while(AfterFliter[0] > 20 || AfterFliter[0] < 0)
		{
			turn_right(100);
		}
	}
	else	//未发现物体,前进
	{
		forward(200);
	}
	vTaskDelay(10); 
}
//物联网控制函数，向服务器发送数据、接收互联网数据控制外设
void net_task(void *pvParameters) 
{
	int temp,tds,storage,voltage;
	float ph;
	int dir;
	int com;
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};								//协议包
	
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
		if(timeCount_date % 500 == 0)   //1000ms / 25 = 10 一秒执行一次（发布温度传感器数据）
		{
			//读出温度数据
			temp = (int)(DS18B20_Get_Temp()*0.1);
			tds = (int)(current_tds());
			ph = current_ph();
			storage = margin_storage_percent();
			voltage = margin_voltage_percent();
			//向缓冲区PUB_BUF1中写入温度数据
			sprintf(PUB_BUF1,"{\"params\":{\"Temp\":%d,\"ph\":%lf,\"tds\":%d,\"storage\":%d,\"voltage\":%d},\"method\":\"thing.event.property.post\"}", 
											temp,ph,tds,storage,voltage);    //发布数据
			//向服务器发布缓冲区PUB_BUF1信息，即发布湿度、温度值
			OneNet_Publish("/sys/gvrxJiLWkq4/Stm32Internet/thing/event/property/post", PUB_BUF1); 
			timeCount_date = 0;
			cmd = ESP8266_GetIPD(3);
			type = MQTT_UnPacketRecv(cmd);
			switch(type)
			{
				case MQTT_PKT_CMD:															//命令下发
					result = MQTT_UnPacketCmd(cmd, &cmdid_topic, &req_payload, &req_len);	//解出topic和消息体
					if(result == 0)
					{
						UsartPrintf(USART_DEBUG, "cmdid: %s, req: %s, req_len: %d\r\n", cmdid_topic, req_payload, req_len);
						MQTT_DeleteBuffer(&mqttPacket);									//删包
					}
				break;
				
				case MQTT_PKT_PUBLISH:														//接收的Publish消息
					result = MQTT_UnPacketPublish(cmd, &cmdid_topic, &topic_len, &req_payload, &req_len, &qos, &pkt_id);
					if(result == 0)
					{
						xTaskNotifyGive(MinorControl_Handler);	//向人为辅助任务发送任务通知，解除其阻塞状态
						UsartPrintf(USART_DEBUG, "topic: %s, topic_len: %d, payload: %s, payload_len: %d\r\n",
																	cmdid_topic, topic_len, req_payload, req_len);
						// 对数据包req_payload进行JSON格式解析
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
							direction.value = item->valueint;		//解析dirction.value信息
							item=cJSON_GetObjectItem(object,"time");
							direction.time = item->valueint;		//解析dirction.time信息	
							dir = direction.value;
							cJSON *object_comeback = cJSON_GetObjectItem(root,"comeback");
							cJSON *item_comeback;
							COMEBACK comeback;
							item_comeback=cJSON_GetObjectItem(object_comeback,"value");
							comeback.value = item_comeback->valueint;		//解析comeback.value信息
							item_comeback=cJSON_GetObjectItem(object_comeback,"time");
							comeback.time = item_comeback->valueint;		//解析comeback.time信息	
							com = comeback.value;
							if(com == 1)		//一旦接到返回信号，立即启动，其优先级最高，不可被其他任务打断
							{
								vTaskResume(Back_Handler);	 //恢复任务
							}
							if(Msg_Queue!=NULL)
							{
								err = xQueueSend(Msg_Queue,&dir,10);
								if(err == errQUEUE_FULL)
								{
									UsartPrintf(USART_DEBUG,"队列已满");
								}
							}
						}
						cJSON_Delete(json);
					}
				break;
			
				case MQTT_PKT_PUBACK:					//发送Publish消息，平台回复的Ack
					if(MQTT_UnPacketPublishAck(cmd) == 0)
						UsartPrintf(USART_DEBUG, "Tips:	MQTT Publish Send OK\r\n");
				break;
					
				case MQTT_PKT_PUBREC:						//发送Publish消息，平台回复的Rec，设备需回复Rel消息
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
					
				case MQTT_PKT_PUBREL:			//收到Publish消息，设备回复Rec后，平台回复的Rel，设备需再回复Comp
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
					
				case MQTT_PKT_PUBCOMP:			//发送Publish消息，平台返回Rec，设备回复Rel，平台再返回的Comp
					if(MQTT_UnPacketPublishComp(cmd) == 0)
					{
						UsartPrintf(USART_DEBUG, "Tips:	Rev PublishComp\r\n");
					}
				break;
			
				case MQTT_PKT_SUBACK:				//发送Subscribe消息的Ack
					if(MQTT_UnPacketSubscribe(cmd) == 0)
						UsartPrintf(USART_DEBUG, "Tips:	MQTT Subscribe OK\r\n");
					else
						UsartPrintf(USART_DEBUG, "Tips:	MQTT Subscribe Err\r\n");
				break;
			
				case MQTT_PKT_UNSUBACK:														//发送UnSubscribe消息的Ack
		
					if(MQTT_UnPacketUnSubscribe(cmd) == 0)
						UsartPrintf(USART_DEBUG, "Tips:	MQTT UnSubscribe OK\r\n");
					else
						UsartPrintf(USART_DEBUG, "Tips:	MQTT UnSubscribe Err\r\n");
				break;

				default:
				result = -1;
				break;
			}
			ESP8266_Clear();									//清空缓存
			if(result == -1)
				return;
			dataPtr = strchr(req_payload, '}');					//搜索'}'
			if(dataPtr != NULL && result != -1)					//如果找到了
			{
				dataPtr++;
				while(*dataPtr >= '0' && *dataPtr <= '9')		//判断是否是下发的命令控制数据
				{
					numBuf[num++] = *dataPtr++;
				}
				num = atoi((const char *)numBuf);				//转为数值形式
			}
			if(type == MQTT_PKT_CMD || type == MQTT_PKT_PUBLISH)
			{
				MQTT_FreeBuffer(cmdid_topic);
				MQTT_FreeBuffer(req_payload);
			}
		}
		if(++timeCount >= 5000)									//发送间隔
		{
			OneNet_Ping();		//保活，否则可能会连接中断
			timeCount=0; 
		}
		else
		{
			vTaskDelay(50);
		}
	}
}
//人为辅助控制任务
void minorcontrol_task(void *pvParameters)
{
	u32 NotifyValue;
	int dir;
	while(1)
	{
		NotifyValue = ulTaskNotifyTake(pdTRUE,portMAX_DELAY);	//此线程只有有控制数据时才触发，其余时间不运行
		if(NotifyValue == 1)  //说明触发了人为辅助控制，此时占用线程
		{
			vTaskSuspend(CollectTask_Handler); //挂起收集任务，等结束后再恢复
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
			vTaskResume(CollectTask_Handler);	 //恢复任务
		}
		else		//一直阻塞
		{
			vTaskDelay(10); 
		}
	}
}
//回到岸边任务
void back_task(void *pvParameters)
{
	turn_left(300);
	delay_ms(2000);
	while(1)
	{
		forward(200);
	}
}
