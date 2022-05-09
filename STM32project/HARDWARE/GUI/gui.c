#include "gui.h"
#include "led.h"
#include "stdlib.h"
#include <stddef.h>
#include <string.h>
#include "stdio.h"
#include "ds18b20.h"
#include "ILI93xx.h"
#include "ff.h"
#include "diskio.h"
#include "stmflash.h"
#include "sensor.h"
/* FreeRTOSͷ�ļ� */
#include "FreeRTOS.h"
#include "task.h"

static char *_acbuffer = NULL;
static char _acBuffer[1024 * 4];

UINT    f_num;
extern FATFS   fs;								/* FatFs�ļ�ϵͳ���� */
extern FIL     file;							/* file objects */
extern FRESULT result; 
extern DIR     dir;

#define ID_FRAMEWIN_0 (GUI_ID_USER + 0x00)
#define ID_GRAPH_0 (GUI_ID_USER + 0x01)
#define ID_Text_0 (GUI_ID_USER + 0x02)
#define ID_Text_1 (GUI_ID_USER + 0x03)
#define ID_Text_2 (GUI_ID_USER + 0x04)
#define ID_Text_3 (GUI_ID_USER + 0x05)
#define ID_Text_4 (GUI_ID_USER + 0x06)
#define ID_Text_5 (GUI_ID_USER + 0x07)
#define ID_Text_6 (GUI_ID_USER + 0x08)
#define ID_Text_7 (GUI_ID_USER + 0x09)
#define ID_Text_8 (GUI_ID_USER + 0x10)
#define ID_Text_9 (GUI_ID_USER + 0x11)
#define ID_Text_10 (GUI_ID_USER + 0x12)
#define ID_BUTTON_0 (GUI_ID_USER + 0x13)
#define ID_BUTTON_1 (GUI_ID_USER + 0x14)
#define ID_BUTTON_2 (GUI_ID_USER + 0x15)
/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
/*���ݶ�����*/
GRAPH_DATA_Handle Graphdata;

short temperature;
extern TaskHandle_t CollectTask_Handler;
extern short DS18B20_Get_Temp();
extern const char MULTIEDIT_title[];
extern const char MULTIEDIT_shebeizhuangtai[];
extern const char MULTIEDIT_shebeizhuangtai_title[];
extern const char MULTIEDIT_yujishiyongsjijian[];
extern const char MULTIEDIT_yujishiyongsjijian_title[];
extern const char MULTIEDIT_shengyudianliang[];
extern const char MULTIEDIT_shengyurongliang[];
extern const char *BUTTON_text[];

/*********************************************************************
*
*       _aDialogCreate����������
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { FRAMEWIN_CreateIndirect, "Framewin", ID_FRAMEWIN_0, 0, 0, 900, 480, 0, 0x0, 0 },
  { GRAPH_CreateIndirect, "Graph", ID_GRAPH_0, 0, 200, 300, 410, 0, 0x0, 0 },
  { TEXT_CreateIndirect,"Text",ID_Text_0,0,0,440,50,0,0x0,0},  //ˮ����������װ��2.0
  { TEXT_CreateIndirect,"Text",ID_Text_1,0,120,440,50,0,0x0,0},  //�豸״̬
  { TEXT_CreateIndirect,"Text",ID_Text_2,200,120,440,50,0,0x0,0},	//����
  { TEXT_CreateIndirect,"Text",ID_Text_3,0,150,440,50,0,0x0,0},	//Ԥ��ʹ��ʱ��
  { TEXT_CreateIndirect,"Text",ID_Text_4,250,150,440,50,0,0x0,0}, 	//1Сʱ
  { TEXT_CreateIndirect,"Text",ID_Text_5,400,0,440,50,0,0x0,0},	//ʣ�����
  { TEXT_CreateIndirect,"Text",ID_Text_6,450,0,440,50,0,0x0,0},	//��ֵ
  { TEXT_CreateIndirect,"Text",ID_Text_7,470,0,440,50,0,0x0,0},	//%
  { TEXT_CreateIndirect,"Text",ID_Text_8,400,120,440,50,0,0x0,0},	//ʣ������
  { TEXT_CreateIndirect,"Text",ID_Text_9,450,120,440,50,0,0x0,0},	//��ֵ
  { TEXT_CreateIndirect,"Text",ID_Text_10,470,120,440,50,0,0x0,0},	//%
  { BUTTON_CreateIndirect, "Button", ID_BUTTON_0, 350, 160, 160, 40, 0, 0x0, 0 },
  { BUTTON_CreateIndirect, "Button", ID_BUTTON_1, 350, 190, 160, 40, 0, 0x0, 0 },
  { BUTTON_CreateIndirect, "Button", ID_BUTTON_2, 350, 220, 160, 40, 0, 0x0, 0 },
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/**
  * @brief �Ի���ص�����
  * @note ��
  * @param pMsg����Ϣָ��
  * @retval ��
  */
static void _cbDialog(WM_MESSAGE* pMsg) {
	WM_HWIN hItem;
    GRAPH_SCALE_Handle hScaleV;
	int     NCode;
	int     Id;
	switch (pMsg->MsgId)
	{
		case WM_INIT_DIALOG:
		/* ��ʼ��Framewin�ؼ� */
		hItem = pMsg->hWin;
		FRAMEWIN_SetText(hItem, "Made by Zhangkaiyang:The sensor");
		FRAMEWIN_SetFont(hItem, GUI_FONT_16B_ASCII);
		/* ��ʼ��Graph�ؼ� */
		hItem = WM_GetDialogItem(pMsg->hWin, ID_GRAPH_0);
		GRAPH_SetColor(hItem, GUI_WHITE, GRAPH_CI_BK);
		GRAPH_SetColor(hItem, GUI_BLACK, GRAPH_CI_GRID);
		GRAPH_SetBorder(hItem, 30, 10, 10, 10);
		GRAPH_SetGridDistX(hItem, 30);
		GRAPH_SetGridDistY(hItem, 30);
		GRAPH_SetLineStyleH(hItem, GUI_LS_DOT);
		GRAPH_SetLineStyleV(hItem, GUI_LS_DOT);
		GRAPH_SetGridVis(hItem, 1);
		/* ������ֱ�̶ȶ��� */
		hScaleV = GRAPH_SCALE_Create(15, GUI_TA_HCENTER | GUI_TA_LEFT,
	                               GRAPH_SCALE_CF_VERTICAL, 50);
		GRAPH_AttachScale(hItem, hScaleV);
		GRAPH_SCALE_SetFactor(hScaleV, 0.1);
		/* �������ݶ��� */
		Graphdata = GRAPH_DATA_YT_Create(GUI_RED, 500, 0, 0);
		GRAPH_AttachData(hItem, Graphdata);
		/*�����ı���ʾ����*/
		/* ��ʼ��Text0 ˮ����������װ��2.0*/
		hItem = WM_GetDialogItem(pMsg->hWin, ID_Text_0);
		TEXT_SetFont(hItem, &FONT_SIYUANHEITI_20_4BPP);
		TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_VCENTER);
		TEXT_SetText(hItem, MULTIEDIT_title);
		/* ��ʼ��Text1 �豸״̬*/
		hItem = WM_GetDialogItem(pMsg->hWin, ID_Text_1);
		TEXT_SetFont(hItem, &FONT_XINSONGTI_16_4BPP);
		TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_VCENTER);
		TEXT_SetText(hItem, MULTIEDIT_shebeizhuangtai);
		/* ��ʼ��Text2 ����*/
		hItem = WM_GetDialogItem(pMsg->hWin, ID_Text_2);
		TEXT_SetFont(hItem, &FONT_XINSONGTI_16_4BPP);
		TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_VCENTER);
		TEXT_SetTextColor(hItem,GUI_GREEN);
		TEXT_SetText(hItem, MULTIEDIT_shebeizhuangtai_title);
		/* ��ʼ��Text3 Ԥ��ʹ��ʱ��*/
		hItem = WM_GetDialogItem(pMsg->hWin, ID_Text_3);
		TEXT_SetFont(hItem, &FONT_XINSONGTI_16_4BPP);
		TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_VCENTER);
		TEXT_SetText(hItem, MULTIEDIT_yujishiyongsjijian);
		/* ��ʼ��Text4 1Сʱ*/
		hItem = WM_GetDialogItem(pMsg->hWin, ID_Text_4);
		TEXT_SetFont(hItem, &FONT_XINSONGTI_16_4BPP);
		TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_VCENTER);
		TEXT_SetTextColor(hItem,GUI_GREEN);
		TEXT_SetText(hItem, MULTIEDIT_yujishiyongsjijian_title);
		/* ��ʼ��Text5 ʣ�����*/
		hItem = WM_GetDialogItem(pMsg->hWin, ID_Text_5);
		TEXT_SetFont(hItem, &FONT_XINSONGTI_16_4BPP);
		TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_VCENTER);
		TEXT_SetText(hItem,  MULTIEDIT_shengyudianliang);
		/* ��ʼ��Text6 ʣ�����-��ֵ*/
		hItem = WM_GetDialogItem(pMsg->hWin, ID_Text_6);
		TEXT_SetFont(hItem, GUI_FONT_8X16X1X2);
		TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_VCENTER);
		TEXT_SetTextColor(hItem,GUI_GREEN);
		TEXT_SetDec(hItem, 0, 5, 0, 0, 0);
		/* ��ʼ��Text7 */
		hItem = WM_GetDialogItem(pMsg->hWin, ID_Text_7);
		TEXT_SetFont(hItem, GUI_FONT_COMIC24B_ASCII);
		TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_VCENTER);
		TEXT_SetTextColor(hItem,GUI_GREEN);
		TEXT_SetText(hItem, "%");
		/* ��ʼ��Text8 ʣ������*/
		hItem = WM_GetDialogItem(pMsg->hWin, ID_Text_8);
		TEXT_SetFont(hItem, &FONT_XINSONGTI_16_4BPP);
		TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_VCENTER);
		TEXT_SetText(hItem,  MULTIEDIT_shengyurongliang);
		/* ��ʼ��Text9 ʣ������-��ֵ*/
		hItem = WM_GetDialogItem(pMsg->hWin, ID_Text_9);
		TEXT_SetFont(hItem, GUI_FONT_8X16X1X2);
		TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_VCENTER);
		TEXT_SetTextColor(hItem,GUI_RED);
		TEXT_SetDec(hItem, 0, 5, 0, 0, 0);
		/* ��ʼ��Text10 */
		hItem = WM_GetDialogItem(pMsg->hWin, ID_Text_10);
		TEXT_SetFont(hItem, GUI_FONT_COMIC24B_ASCII);
		TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_VCENTER);
		TEXT_SetTextColor(hItem,GUI_RED);
		TEXT_SetText(hItem, "%");
		/* ��ʼ��Button0 */
		hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_0);
		BUTTON_SetFont(hItem, &FONT_SIYUANHEITI_20_4BPP);
		BUTTON_SetText(hItem, BUTTON_text[0]);
		/* ��ʼ��Button1 */
		hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_0);
		BUTTON_SetFont(hItem, &FONT_SIYUANHEITI_20_4BPP);
		BUTTON_SetText(hItem, BUTTON_text[1]);
		/* ��ʼ��Button2 */
		hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_0);
		BUTTON_SetFont(hItem, &FONT_SIYUANHEITI_20_4BPP);
		BUTTON_SetText(hItem, BUTTON_text[2]);
		break;
	case WM_NOTIFY_PARENT:
	  Id = WM_GetId(pMsg->hWinSrc);
	  NCode = pMsg->Data.v;
	  switch (Id) 
	  {
		case ID_BUTTON_0: // Notifications sent by 'Button0'
			switch (NCode) 
			{
				case WM_NOTIFICATION_CLICKED:
				break;
				case WM_NOTIFICATION_RELEASED:
				vTaskResume(CollectTask_Handler);		//ʹ��������
				break;
			}
		break;
		case ID_BUTTON_1: // Notifications sent by 'Button1'
			switch (NCode) 
			{
				case WM_NOTIFICATION_CLICKED:
				break;
				case WM_NOTIFICATION_RELEASED:
				vTaskSuspend(CollectTask_Handler);		//ʹ���������ƶ�
				break;
			}
		break;
		case ID_BUTTON_2: // Notifications sent by 'Button2'
			switch (NCode) 
			{
				case WM_NOTIFICATION_CLICKED:
				break;
				case WM_NOTIFICATION_RELEASED:
				LED1=~LED1;		//�㵹����
				break;
			}
		break;
	 }
	default:
		WM_DefaultProc(pMsg);
		break;
	}
}


/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
 /**
  * @brief �ԶԻ���ʽ��Ӵ����ؼ�
  * @note ��
  * @param ��
  * @retval hWin����Դ���е�һ���ؼ��ľ��
  */
WM_HWIN CreateFramewin(void);  
WM_HWIN CreateFramewin(void) {
	WM_HWIN hWin;
	hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
	return hWin;
}

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/**
  * @brief �Ӵ洢���ж�ȡ����
  * @note ��
  * @param 
  * @retval NumBytesRead���������ֽ���
  */
int _GetData(void * p, const U8 ** ppData, unsigned NumBytesReq, U32 Off)
{
	static int FileAddress = 0;
	UINT NumBytesRead;
	FIL *Picfile;
	
	Picfile = (FIL *)p;
	
	if(NumBytesReq > sizeof(_acBuffer))
	{NumBytesReq = sizeof(_acBuffer);}
	
	if(Off == 1) FileAddress = 0;
	else FileAddress = Off;
	result = f_lseek(Picfile, FileAddress);
	
	/* �����ٽ�� */
	taskENTER_CRITICAL();
	result = f_read(Picfile, _acBuffer, NumBytesReq, &NumBytesRead);
	/* �˳��ٽ�� */
	taskEXIT_CRITICAL();
	
	*ppData = (const U8 *)_acBuffer;
	
	return NumBytesRead;
}

/*
	������ʾ
*/
/* �ֿ�ṹ�� */
GUI_FONT     	FONT_SIYUANHEITI_20_4BPP;
GUI_FONT     	FONT_XINSONGTI_16_4BPP;

/* �ֿ⻺���� */
uint8_t *SIFbuffer20;
uint8_t *SIFbuffer16;
static const char FONT_STORAGE_ROOT_DIR[]  =   "0:";
static const char FONT_XINSONGTI_16_ADDR[] = 	 "0:/Font/������16_4bpp.sif";
static const char FONT_SIYUANHEITI_20_ADDR[] = 	 "0:/˼Դ����20_4bpp.sif";

/* �洢����ʼ����־ */
static uint8_t storage_init_flag = 0;

/* �ֿ�洢���ļ�ϵͳʱ��Ҫʹ�õı��� */
static FIL fnew;									  /* file objects */
static FATFS fs;									  /* Work area (file system object) for logical drives */
static FRESULT res;
static UINT br;            			    /* File R/W count */

/**
  * @brief  �����������ݵ�SDRAM
  * @note ��
  * @param  res_name��Ҫ���ص��ֿ��ļ���
  * @retval Fontbuffer���Ѽ��غõ��ֿ�����
  */
void *FONT_SIF_GetData(const char *res_name)
{
	uint8_t *Fontbuffer;
	GUI_HMEM hFontMem;
	if (storage_init_flag == 0)
	{
		/* ����sd���ļ�ϵͳ */
		res = f_mount(&fs,FONT_STORAGE_ROOT_DIR,1);
		storage_init_flag = 1;
	}
	
	/* ���ֿ� */
	res = f_open(&fnew , res_name, FA_OPEN_EXISTING | FA_READ);
	if(res != FR_OK)
	{
		printf("Open font failed! res = %d\r\n", res);
		while(1);
	}
	
	/* ����һ�鶯̬�ڴ�ռ� */
	hFontMem = GUI_ALLOC_AllocZero(fnew.fsize);
	/* ת����̬�ڴ�ľ��Ϊָ�� */
	Fontbuffer = GUI_ALLOC_h2p(hFontMem);

	/* ��ȡ���� */
	res = f_read(&fnew, Fontbuffer, fnew.fsize, &br);
	if(res != FR_OK)
	{
		printf("Read font failed! res = %d\r\n", res);
		while(1);
	}
	f_close(&fnew);
	
	return Fontbuffer;  
}

/**
  * @brief  ����SIF����
  * @param  ��
  * @retval ��
  */
void Create_SIF_Font(void) 
{
	/* ��ȡ�������� */
	SIFbuffer16 = FONT_SIF_GetData(FONT_XINSONGTI_16_ADDR);
	SIFbuffer20 = FONT_SIF_GetData(FONT_SIYUANHEITI_20_ADDR);
	
	/* ������18 */
	GUI_SIF_CreateFont(SIFbuffer16,               /* �Ѽ��ص��ڴ��е��������� */
	                   &FONT_XINSONGTI_16_4BPP,   /* GUI_FONT ����ṹ��ָ�� */
										 GUI_SIF_TYPE_PROP_AA4_EXT);/* �������� */
	/* ˼Դ����36 */
	GUI_SIF_CreateFont(SIFbuffer20,               /* �Ѽ��ص��ڴ��е��������� */
	                   &FONT_SIYUANHEITI_20_4BPP, /* GUI_FONT ����ṹ��ָ�� */
										 GUI_SIF_TYPE_PROP_AA4_EXT);/* �������� */
}
/**
  * @brief GUI������
  * @note ��
  * @param ��
  * @retval ��
  */
void MainTask(void)
{
	/* �������� */
	WM_HWIN hWin;
    WM_HWIN hItem;

	int voltage,storage;
	/* ����UTF-8���� */
	GUI_UC_SetEncodeUTF8();
//	/* �������� */
	Create_SIF_Font();
	hWin = CreateFramewin();

	voltage = margin_voltage_percent();
	storage = margin_storage_percent();
	while(1)
	{
		/* ��GRAPH���ݶ���������� */
		GRAPH_DATA_YT_AddValue(Graphdata, DS18B20_Get_Temp());
		hItem = WM_GetDialogItem(hWin, ID_Text_6);   //���վ�������µ���
		TEXT_SetDec(hItem, voltage, 2, 0, 0, 0);		//����
		hItem = WM_GetDialogItem(hWin, ID_Text_9);   //�����������������
		TEXT_SetDec(hItem, storage, 2, 0, 0, 0);  //����
		GUI_Delay(150);
	}
}
