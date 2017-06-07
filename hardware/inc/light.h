#ifndef _LIGHT_H_
#define _LIGHT_H_

#include "stm32f10x.h"



#define LIGHT_ON GPIO_SetBits(GPIOA,GPIO_Pin_15)
#define LIGHT_OFF GPIO_ResetBits(GPIOA,GPIO_Pin_15)

#define  LIGHT_1    0X01
#define  L_ON  0
#define  L_OFF  (!L_ON)

typedef struct
{

	_Bool LightSta;

} LIGHT_STATUS;

 extern LIGHT_STATUS lightStatus;


void LIGHT_Init(void);
 
void LIGHT_Switch(const uint8_t statu,const uint8_t mode);
_Bool LIGHT_GetValue(void);



#endif