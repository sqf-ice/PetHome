#include "rthongwai.h"
#include "tcrt5000.h"
volatile unsigned char  body_value=0;

void Body_Init(void){
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(Body_GPIO_CLK,ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;//感应高电平，所以选择下拉
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	
	GPIO_InitStructure.GPIO_Pin = Body_GPIO_PIN;
	GPIO_Init(Body_GPIO_PORT,&GPIO_InitStructure);
}



//void Get_Bodystatus(void){
//	if(GPIO_ReadInputDataBit(Body_GPIO_PORT,Body_GPIO_PIN)){
//		TIM3->CCR1= 300;
//		Led5_Set(LED_ON);
//	}else{
//		Led5_Set(LED_OFF);
//		TIM3->CCR1= 735;
//	}
//}