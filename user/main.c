//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"

//����Э���
#include "onenet.h"
#include "fault.h"

//�����豸
#include "net_device.h"

//Ӳ������
#include "led.h"
#include "delay.h"
#include "lcd1602.h"
#include "usart.h"
#include "hwtimer.h"
#include "i2c.h"
#include "gy30.h"
#include "adxl345.h"
#include "sht20.h"
#include "iwdg.h"
#include "at24c02.h"
#include "selfcheck.h"
#include "beep.h"
#include "info.h"
#include "tcrt5000.h"
#include "rthongwai.h"
#include "duoji.h"

//C��
#include <string.h>


//������
DATA_STREAM dataStream[] = {
								{"Red_Led", &ledStatus.Led4Sta, TYPE_BOOL, 1},
								{"Green_Led", &ledStatus.Led5Sta, TYPE_BOOL, 1},
								{"Yellow_Led", &ledStatus.Led6Sta, TYPE_BOOL, 1},
								{"Blue_Led", &ledStatus.Led7Sta, TYPE_BOOL, 1},
								{"beep", &beepInfo.Beep_Status, TYPE_BOOL, 1},
								{"temperature", &sht20Info.tempreture, TYPE_FLOAT, 1},
								{"humidity", &sht20Info.humidity, TYPE_FLOAT, 1},
								{"Xg", &adxlInfo.incidence_Xf, TYPE_FLOAT, 1},
								{"Yg", &adxlInfo.incidence_Yf, TYPE_FLOAT, 1},
								{"Zg", &adxlInfo.incidence_Zf, TYPE_FLOAT, 1},
								{"errType", &faultTypeReport, TYPE_UCHAR, 1},
							};
unsigned char dataStreamLen = sizeof(dataStream) / sizeof(dataStream[0]);





/*
************************************************************
*	�������ƣ�	Hardware_Init
*
*	�������ܣ�	Ӳ����ʼ��
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		��ʼ����Ƭ�������Լ�����豸
************************************************************
*/
void Hardware_Init(void)
{
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);								//�жϿ�������������

	Delay_Init();																//systick��ʼ��
	
	Led_Init();																	//LED��ʼ��
	
	Body_Init();																//��������ʼ��
	
	Duoji_Init(9999,143);	 											//�����ʼ�� 
	
	Beep_Init();																//��������ʼ��
	
	TCRT5000_Init();															//TCRT5000��ʼ��
	
	IIC_Init();																	//����IIC���߳�ʼ��
	
	Lcd1602_Init();																//LCD1602��ʼ��
	
	Usart1_Init(115200); 														//��ʼ������   115200bps
	
	Lcd1602_DisString(0x80, "Check Power On");									//��ʾ���п������
	Check_PowerOn(); 															//�ϵ��Լ�
	Lcd1602_Clear(0x80);														//���һ����ʾ
	
	if(checkInfo.ADXL345_OK == DEV_OK) 											//�����⵽ADXL345���ʼ��
		ADXL345_Init();
	


	if(RCC_GetFlagStatus(RCC_FLAG_IWDGRST) == SET) 								//����ǿ��Ź���λ����ʾ
	{
		UsartPrintf(USART_DEBUG, "WARN:	IWDG Reboot\r\n");
		
		RCC_ClearFlag();														//������Ź���λ��־λ
		
		faultTypeReport = faultType = FAULT_REBOOT; 							//���Ϊ��������
		
		netDeviceInfo.reboot = 1;
	}
	else
	{
//		UsartPrintf(USART_DEBUG, "2.DEVID: %s,     APIKEY: %s\r\n"
//								, oneNetInfo.devID, oneNetInfo.apiKey);
		
		netDeviceInfo.reboot = 0;
	}
	
	//Iwdg_Init(4, 1250); 														//64��Ƶ��ÿ��625�Σ�����1250�Σ�2s
	
	Lcd1602_Clear(0xff);														//����
	
	Timer6_7_Init(TIM6, 49, 35999);												//72MHz��36000��Ƶ-500us��50����ֵ�����ж�����Ϊ500us * 50 = 25ms
	Timer6_7_Init(TIM7, 1999, 35999);											//72MHz��36000��Ƶ-500us��2000����ֵ�����ж�����Ϊ500us * 2000 = 1s
																				//��ʱ�������״̬��־λ
	
	UsartPrintf(USART_DEBUG, "3.Hardware init OK\r\n");							//��ʾ��ʼ�����

}

/*
************************************************************
*	�������ƣ�	main
*
*	�������ܣ�	
*
*	��ڲ�����	��
*
*	���ز�����	0
*
*	˵����		
************************************************************
*/
int main(void)
{
	
	unsigned char *dataPtr;
	unsigned int runTime = 0;
	_Bool sendFlag = 0;

	Hardware_Init();									//Ӳ����ʼ��
	

	
	NET_DEVICE_IO_Init();								//�����豸IO��ʼ��
	NET_DEVICE_Reset();									//�����豸��λ
	NET_DEVICE_Set_DataMode(DEVICE_CMD_MODE);			//����Ϊ�����շ�ģʽ(����ESP8266Ҫ����AT�ķ��ػ���ƽ̨�·����ݵķ���)
	
	while(1)
	{
		
		if(oneNetInfo.netWork == 1)
		{

/******************************************************************************
			����������
******************************************************************************/
			if(timInfo.timer6Out - runTime >= 10)									//25sһ��(25ms�ж�)
			{
				runTime = timInfo.timer6Out;
				
				if(sendFlag)
				{
					TIM_Cmd(OS_TIMER, DISABLE);
					OneNet_Status();												//״̬���
					TIM_Cmd(OS_TIMER, ENABLE);
				}
				else
				{
					TIM_Cmd(OS_TIMER, DISABLE);
					OneNet_SendData(HTTP_TYPE3, dataStreamLen);						//���ݷ���
					TIM_Cmd(OS_TIMER, ENABLE);
				}
		
				sendFlag = !sendFlag;
			}
			
/******************************************************************************
			���ͽ������
******************************************************************************/
			if(oneNetInfo.netWork && NET_DEVICE_Get_DataMode() == DEVICE_DATA_MODE)	//�������� �� ���������ģʽʱ
			{
				dataPtr = NET_DEVICE_GetIPD(0);										//���ȴ�����ȡƽ̨�·�������
				if(dataPtr != NULL)													//�������ָ�벻Ϊ�գ�������յ�������
				{
					OneNet_Event(dataPtr);											//���д���
				}
			}
			
/******************************************************************************
			������
******************************************************************************/
			if(checkInfo.ADXL345_OK == DEV_OK) 										//ֻ���豸����ʱ���Ż��ȡֵ����ʾ
			{
				ADXL345_GetValue();													//�ɼ�����������
				Lcd1602_DisString(0x80, "X%0.1f,Y%0.1f,Z%0.1f", adxlInfo.incidence_Xf, adxlInfo.incidence_Yf, adxlInfo.incidence_Zf);
			}
			if(checkInfo.SHT20_OK == DEV_OK) 										//ֻ���豸����ʱ���Ż��ȡֵ����ʾ
			{
				SHT20_GetValue();													//�ɼ�����������
				Lcd1602_DisString(0xC0, "%0.1fC,%0.1f%%", sht20Info.tempreture, sht20Info.humidity);
			}
			//����
			if(t5000Info.status == TCRT5000_ON)
			{
				TCRT5000_GetValue(5);
				if(t5000Info.voltag < 3500)
					Led4_Set(LED_ON);
				else
					Led4_Set(LED_OFF);
			}
			
//			Get_Bodystatus();//��������ж�
			if(GPIO_ReadInputDataBit(Body_GPIO_PORT,Body_GPIO_PIN)||(t5000Info.voltag < 3500)){
				TIM3->CCR1= 300;
				Led5_Set(LED_OFF);
			}else{
				TIM3->CCR1= 735;
				Led5_Set(LED_ON);
			}
			
/******************************************************************************
			������
******************************************************************************/
			if(faultType != FAULT_NONE)												//��������־������
			{
				UsartPrintf(USART_DEBUG, "WARN:	Fault Process\r\n");
				Fault_Process();													//�������������
			}
			
/******************************************************************************
			���ݷ���
******************************************************************************/
			if(oneNetInfo.sendData)
			{
				oneNetInfo.sendData = OneNet_SendData(HTTP_TYPE3, dataStreamLen);
			}
		}
		else
		{
/******************************************************************************
			��ʼ�������豸������ƽ̨
******************************************************************************/
			if(!oneNetInfo.netWork && (checkInfo.NET_DEVICE_OK == DEV_OK))			//��û������ �� ����ģ���⵽ʱ
			{
				NET_DEVICE_Set_DataMode(DEVICE_CMD_MODE);							//����Ϊ�����շ�ģʽ
				
				if(!NET_DEVICE_Init(oneNetInfo.ip, oneNetInfo.port))				//��ʼ�������豸������������
				{
					UsartPrintf(USART_DEBUG, "Tips:	NetWork OK\r\n");
		
					oneNetInfo.netWork = 1;
					
					NET_DEVICE_Set_DataMode(DEVICE_DATA_MODE);						//�����豸ָ��ģʽ
					
					DelayXms(200);

					runTime = timInfo.timer6Out;
				}
			}
			
/******************************************************************************
			�����豸���
******************************************************************************/
			if(checkInfo.NET_DEVICE_OK == DEV_ERR) 									//�������豸δ�����
			{
				NET_DEVICE_Set_DataMode(DEVICE_CMD_MODE);							//����Ϊ�����շ�ģʽ
				
				if(timerCount >= NET_TIME) 											//����������ӳ�ʱ
				{
					UsartPrintf(USART_DEBUG, "Tips:		Timer Check Err\r\n");
					
					NET_DEVICE_Reset();												//��λ�����豸
					timerCount = 0;													//�������ӳ�ʱ����
					faultType = FAULT_NONE;											//��������־
				}
				
				if(!NET_DEVICE_Exist())												//�����豸���
				{
					UsartPrintf(USART_DEBUG, "NET Device :Ok\r\n");
					checkInfo.NET_DEVICE_OK = DEV_OK;								//��⵽�����豸�����
					NET_DEVICE_Set_DataMode(DEVICE_DATA_MODE);						//����Ϊ�����շ�ģʽ
				}
				else
					UsartPrintf(USART_DEBUG, "NET Device :Error\r\n");
			}
		}
	}

}