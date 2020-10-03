/***************************************
 * 文件名  ：usart1.c
 * 描述    ：配置USART1         
 * 实验平台：MINI STM32开发板 基于STM32F103C8T6
 * 硬件连接：------------------------
 *          | PA9  - USART1(Tx)      |
 *          | PA10 - USART1(Rx)      |
 *           ------------------------
 * 库版本  ：ST3.0.0  

**********************************************************************************/

#include "usart1.h"
#include <stdarg.h>
#include "misc.h"
#include "ESP8266.h"
#include "stm32f10x_it.h"

void USART1_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
	
	
	
	/* 使能 USART1 时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE); 

	/* USART1 使用IO端口配置 */    
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //复用推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);    
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);   //初始化GPIOA
	 
 
	
	
	/* USART1 工作模式配置 */
	USART_InitStructure.USART_BaudRate = 115200;	//波特率设置：115200
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//数据位数设置：8位
	USART_InitStructure.USART_StopBits = USART_StopBits_1; 	//停止位设置：1位
	USART_InitStructure.USART_Parity = USART_Parity_No ;  //是否奇偶校验：无
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//硬件流控制模式设置：没有使能
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//接收与发送都使能
	USART_Init(USART1, &USART_InitStructure);  //初始化USART1
	

	//USART_ClearITPendingBit(USART1, USART_IT_RXNE); //清除中断标志位
	
	/*NVIC配置*/
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; 
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
  NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART1, ENABLE);// USART1使能
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	USART_ITConfig(USART1,USART_IT_IDLE,ENABLE);
	USART_ClearFlag(USART1,USART_FLAG_TC);
	
	
}

 /*发送一个字节数据*/
 void UART1SendByte(unsigned char SendData)
{	   
        USART_ClearFlag(USART1,USART_FLAG_TC);
	      USART_SendData(USART1,SendData);
        while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);	
        USART_ClearFlag(USART1,USART_FLAG_TC);	
}  

/*接收一个字节数据*/
unsigned char UART1GetByte(unsigned char* GetData)
{   	   
        if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
        {  return 0;//没有收到数据 
		}
        *GetData = USART_ReceiveData(USART1); 
        return 1;//收到数据
}
/*接收一个数据，马上返回接收到的这个数据*/
void UART1Test(void)
{
       unsigned char i = 0;

       while(1)
       {    
		 while(UART1GetByte(&i))
        {
         USART_SendData(USART1,i);
        }      
       }     
}

void Usart_SendString(USART_TypeDef* USARTx,char *str)
{
  
   while(*str){
      UART1SendByte(*str++);    
}
   
}

void Usart1_SendU8Array(u8*buf, int buflen){
   int t=0;
	 for(t=0;t<buflen;t++)
	     UART1SendByte(buf[t]);
}


	
