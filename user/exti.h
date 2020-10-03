#ifndef __EXTI_H
#define	__EXTI_H	   
#include "stm32f10x.h"
#include "stm32f10x_exti.h"

void EXTI_Configuration(void);
void EXTI0_IRQHandler(void) ;
u8 read_key_down(void);

#endif
