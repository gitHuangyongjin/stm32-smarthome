#include "stm32f10x_exti.h"
#include "exti.h"
#include "misc.h"
#include "key.h"
#include "SysTick.h"


u8 key_down=0;

void EXTI_Configuration(){
   EXTI_InitTypeDef EXTI_InitStruct;
	 NVIC_InitTypeDef NVIC_InitStruct;
	
	 Key_GPIO_Config();
	

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);     
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource0);
	
	
	EXTI_InitStruct.EXTI_Line = EXTI_Line0;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	
	EXTI_Init(&EXTI_InitStruct);
	
			
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn; 
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	
	NVIC_Init(&NVIC_InitStruct);
	
}

void EXTI0_IRQHandler() 
{       
        Delay_ms(10);                                          
        if(Key_Scan(GPIOA,0)==1)                                             
        {     
					key_down=1; 
        }                 
        EXTI_ClearITPendingBit(EXTI_Line0);  
}

u8 read_key_down(){
    return key_down;
}
