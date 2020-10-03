#include "adc.h"
#include "dma.h"
#include "stm32f10x_dma.h"


volatile uint16_t AD_Value[2]; //用来存放ADC转换结果，也是DMA的目标地址
int i;

u16 DMA1_MEM_LEN;//保存DMA每次数据传送的长度 	    
 
void DMA_Configuration(void)
{
	DMA_InitTypeDef DMA_InitStructure;
 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA传输
	
	
	DMA_DeInit(DMA1_Channel1); //将DMA的通道1寄存器重设为缺省值
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&ADC1->DR; //DMA外设ADC基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&AD_Value; //DMA内存基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; //内存作为数据传输的目的地
	DMA_InitStructure.DMA_BufferSize = 2; //DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //数据宽度为16位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //数据宽度为16位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //工作在循环缓存模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMA通道 x拥有高优先级
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; //DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA1_Channel1, &DMA_InitStructure); //根据DMA_InitStruct中指定的参数初始化DMA的通道
	
	DMA_Cmd(DMA1_Channel1, ENABLE); //启动DMA通道

} 
//开启一次DMA传输
void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx)
{ 
	DMA_Cmd(DMA_CHx, DISABLE );  //关闭USART1 TX DMA1 所指示的通道      
 	//DMA_SetCurrDataCounter(DMA1_Channel4);//,DMA1_MEM_LEN);//DMA通道的DMA缓存的大小
 	DMA_Cmd(DMA_CHx, ENABLE);  //使能USART1 TX DMA1 所指示的通道 
}	  


float Getvolt(u8 channel)   

  {
   
		if(channel==8)
			return (float)AD_Value[0]*3.3/4096;
		else if(channel==9)
			return (float)AD_Value[1]*3.3/4096;
		else 
      return (u16)(AD_Value[0] * 330 / 4096);   //求的结果扩大了100倍，方便下面求出小数

  }


float Read_soil(){

	  float soil;
	  soil=(-Getvolt(8)*100+340)/2;
	  if(soil<=0)return 0;
	  else if(soil>100)return 100;
	  else 
	  return soil;
}


float Read_lux(){

	  float lux;
	  lux=(-Getvolt(9)*100+337)*3;
	  if(lux<=0)return 0;
	  else if(lux>1000)return 1000;
	  else 
	  return lux;
}
















