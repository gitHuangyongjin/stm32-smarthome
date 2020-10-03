/******************* ********************
 * ÎÄ¼þÃû  £ºled.c
 * ÃèÊö    £ºled Ó¦ÓÃº¯Êý¿â
 *          
 * ÊµÑéÆ½Ì¨£ºMINI STM32¿ª·¢°å »ùÓÚSTM32F103C8T6
 * Ó²¼þÁ¬½Ó£º-----------------
 *          |   PC13 - LED1   |
 *          |       
 *          |                 |
 *           ----------------- 
 * ¿â°æ±¾  £ºST3.0.0  																										  
*********************************************************/
#include "led.h"


 /***************  ÅäÖÃLEDÓÃµ½µÄI/O¿Ú *******************/
void LED_GPIO_Config(void)	
{
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE); // Ê¹ÄÜPC¶Ë¿ÚÊ±ÖÓ  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;	//Ñ¡Ôñ¶ÔÓ¦µÄÒý½Å
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);  //³õÊ¼»¯PC¶Ë¿Ú
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;	//Ñ¡Ôñ¶ÔÓ¦µÄÒý½
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    
	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);  //³õÊ¼»¯PC¶Ë¿Ú
  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;	//Ñ¡Ôñ¶ÔÓ¦µÄÒý½Å
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    
	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);  //³õÊ¼»¯PC¶Ë¿Ú
	
  GPIO_ResetBits(GPIOA, GPIO_Pin_1 );	 
	GPIO_ResetBits(GPIOA, GPIO_Pin_2 );	
	GPIO_ResetBits(GPIOA, GPIO_Pin_3 );	
	
	
}



