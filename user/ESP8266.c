#include "stm32f10x.h"
#include "usart1.h"
#include <string.h>
#include <stdio.h>
#include "ESP8266.h"
#include "SysTick.h"
#include "dma.h"
#include "led.h"
#include "stm32f10x_it.h"


int to_index=0;  //全局变量，用于mqtt接口的接收数据函数transport_getdata()
void initCGQ(void) //传感器结构体
{
	
	cgq_proper.Buzzer= FALSE;
	cgq_proper.LightSwitch= FALSE;  
	cgq_proper.shuibengzhuangtai= FALSE;
	cgq_proper.Env_lux=100;   //需要7位
	cgq_proper.soilHumidity=60.0;   
	
	cgq_proper.CombustibleGasCheck=20.00; //需要 7位
	cgq_proper.Temperature=29.2;
	cgq_proper.chuangliankaiguan= FALSE;
	
}

/*
 * 功能：串口1中断服务函数；
 * 参数：None
 * 返回值：None 
*/ 
void USART1_IRQHandler(void){
	  static u16  i=0;
  if(USART_GetITStatus(USART1, USART_IT_RXNE) ) 
  {
    RxBuffer1[i++] = USART_ReceiveData(USART1);  
    if(i==RxBuffer_size){
		   i = RxBuffer_size-1;
		}
	
	}
  if(USART_GetITStatus(USART1, USART_IT_IDLE))                   
  { 

    USART_ReceiveData(USART1); 
		i=0;
		
  }
} 



//查找字符串
/*
 * 功能：查找字符串是否包含一个字符串
 * 参数
 *       dest：待查找目标字符串
 *       src：
 *       retry_cn ：查询超时时间
 * 返回  
 *       1成功 0失败
 * 说明 
 *       当发出一个AT指令后，需要一段时间等待8266回复 
 *       这个时间通常是几百ms  …… 
*/

static u8 findstr(char * dest,char * src,u16 retry_cn){
   u16 retry =retry_cn;
	 u8 result_flag=0;
	
	while(strstr(dest,src)==0 && --retry!=0){
   Delay_ms(10);
}
	if(retry==0){
	  return 0;
	}
  result_flag=1;
	
	if(result_flag){
	   return 1;
	}else{
	   return 0;
	}
		
}


u8 checkESP8266(void){
   memset(RxBuffer1,0,RxBuffer_size);
	 Usart_SendString(USART1,"AT\r\n");
	if(findstr(RxBuffer1,"OK",200)!=0){
		return 1;
	}else{
		return 0;
	}
	
}

//initESP8266
/*
 * 
*/
u8 initESP8266(void){
   Usart_SendString(USART1,"+++"); //退出透传
	 Delay_ms(500);
	 Usart_SendString(USART1,"AT+RST\r\n");  //重启
	 Delay_ms(1000);
	if(checkESP8266()==0){
		return 0; 
	}	
	memset(RxBuffer1,0,RxBuffer_size);
	Usart_SendString(USART1,"ATE0\r\n");
	if(findstr(RxBuffer1,"OK",200)==0){
	   memset(RxBuffer1,0,RxBuffer_size);
		 return 0;
	}
	return 1;
}



void restoreESP8266(void){
   Usart_SendString(USART1,"+++");
	 Delay_ms(500);
	 Usart_SendString(USART1,"AT+RESTORE\r\n");
	 NVIC_SystemReset();  //同时重启单片机;
}


/*
 * 返回值：非零成功
*/
u8 connectAP(char* ssid, char* pwd){
   memset(RxBuffer1,0,RxBuffer_size);
	 Usart_SendString(USART1,"AT+CWMODE?\r\n");
	 if(findstr(RxBuffer1,"CWMODE:1",200)==0){
	    memset(RxBuffer1,0,RxBuffer_size);
		  Usart_SendString(USART1,"AT+CWMODE_CUR=1\r\n");
		  if(findstr(RxBuffer1,"OK",200)==0){
			    return 0;
			}
	 }
	 
	 memset(TxBuffer1,0,RxBuffer_size);
	 memset(RxBuffer1,0,RxBuffer_size);
	 sprintf(TxBuffer1,"AT+CWJAP_CUR=\"%s\",\"%s\"\r\n",ssid,pwd);
	 Usart_SendString(USART1,TxBuffer1);
	 if(findstr(RxBuffer1,"OK",800)!=0){
	    return 1;
	 }
	 return 0;
}

/*
 * 功能： 使用指定协议连接到服务器
 * 参数：
 *        mode:协议类型"TCP"/"UDP"
 *        
 * 返回值：
           1成功，0失败
   说明：  
           失败原因有以下几种（USART、ESP8266正常的情况下）
           1. ip、port有误
           2. 未连接AP
           3. 服务器禁止添加
*/
u8 connectServer(char* mode,char*  ip,u16 port){
   memset(RxBuffer1,0,RxBuffer_size);
	 memset(TxBuffer1,0,RxBuffer_size);
	 
	 Usart_SendString(USART1,"+++");          ///先退出透传;
	 Delay_ms(500);
	/*格式化待发送AT命令*/
	 sprintf(TxBuffer1,"AT+CIPSTART=\"%s\",\"%s\",%d\r\n",mode,ip,port);
	Usart_SendString(USART1,TxBuffer1);
	if(findstr(RxBuffer1,"CONNECT",800)!=0){
	    memset(RxBuffer1,0,RxBuffer_size);
		  Usart_SendString(USART1,"AT+CIPMODE=1\r\n");  //透传模式;
		  if(findstr(RxBuffer1,"OK",200)!=0){
			   memset(RxBuffer1,0,RxBuffer_size);
				 Usart_SendString(USART1,"AT+CIPSEND\r\n");  //发送
				 if(findstr(RxBuffer1,">",200)!=0){
				     return 1;
				 }else{
				     return 0;
				 }
				 
			}else{
			    return 0;
				
			}
	}else{
	     return 0;
	}
}

/*
 * 功能：主动和服务器断开连接
   参数：None
   返回值：
         非零断开成功，0断开失败
*/
u8 disconnectServer(void){
    Usart_SendString(USART1,"+++");   //退出透传
	  Delay_ms(300);
	  memset(RxBuffer1,0,RxBuffer_size);
	  Usart_SendString(USART1,"AT+CIPCLOSE\r\n"); 
	  
	  if(findstr(RxBuffer1,"CLOSED",200)!=0) {
		   return 1;
		}else return 0;
}

/*
  功能： 透传模式下的数据发送函数
  参数： 
   
   返回： None
*/
void sendBuffertoServer(char* buffer){
    //memset(RxBuffer1,0,RxBuffer_size);
    Usart_SendString(USART1,buffer);	
}
/*
  功能：移植mqtt实现发送数据接口
  参数： 
   
   返回： int
*/
int transport_sendPacketBuffer(const char*server_ip_and_port,unsigned char* buf, int buflen){  
   
	 memset(RxBuffer1,0,RxBuffer_size);
	 Usart_SendString(USART1,"+++");          ///先退出透传;
	 Delay_ms(500);
   Usart_SendString(USART1,"AT+CIPSTATUS\r\n");//向esp8266查询网络状态，是否连接了服务器
	 
	 if(findstr(RxBuffer1,"STATUS:3",200)!= NULL){  //已经建立tcp
		//开启透传
		memset(RxBuffer1,0,RxBuffer_size);
		Usart_SendString(USART1,"AT+CIPMODE=1\r\n");  //透传模式;
		 
		if(findstr(RxBuffer1,"OK",200)!= NULL){
		     memset(RxBuffer1,0,RxBuffer_size);
				 Usart_SendString(USART1,"AT+CIPSEND\r\n");  //发送
			   
			   if(findstr(RxBuffer1,">",200)!=NULL){
					 //可以发送数据了
					 memset(RxBuffer1,0,RxBuffer_size);
					 Usart1_SendU8Array(buf,buflen);  
					 memset(buf,0,MQTT_MAX_BUF_SIZE);
					 //将数据下标设为0
					 to_index=0;
					 //Delay_ms(1000);  ///这个要调试
				   return 1;
	    	}
				 //建立透传失败
				 else return 0;
		
	  }
		//建立透传失败
	  else return 0;
	 
 }
		//如果没有建立连接,就建立阿里云tcp
	  if(connectServer(MODE,IP,PORT)!=0){
			  memset(RxBuffer1,0,RxBuffer_size);
		    Usart1_SendU8Array(buf,buflen);
			 memset(buf,0,MQTT_MAX_BUF_SIZE);
			  to_index=0;
		    return 1;
		}
		//不能建立tcp;
		else{ return 0;}
		
}  
/*
  功能：移植mqtt实现接收数据接口
  参数： 
   
   返回： int
*/

int transport_getdata(unsigned char*buf, int count){
    
	  
	  //char*Reb =&RxBuffer1[0];
	  //int count2=strlen(Reb);
	 ///这里影响太大了，
	  memcpy(buf,&RxBuffer1[to_index],count);
	  to_index+=count;
	 
	 //接收成功的意思
	 return count;
    //count+=count2;
	  
}

/*近场通信获取接收数据，暂定100个字节的接收*/
void Near_transport_getdata(unsigned char*buf){
    
   memcpy(buf,&RxBuffer1,100);
	 memset(RxBuffer1,0,RxBuffer_size);
}


u8 led=0,shuibeng=0,fengming=0;

/*近场通信处理接收的命令*/
void Near_processServer(unsigned char*buf){
  
  if(strstr((const char*)buf,"ACL")!=NULL){ //LED
	    if(led%2==0){
	       LED(ON);
				 cgq_proper.LightSwitch=TRUE;
		  }
			else if(led%2==1){
			   LED(OFF);
				 cgq_proper.LightSwitch=FALSE;
				
			}
			++led;
	}

	if(strstr((const char*)buf,"ACF")!=NULL){	//水泵
	    if(shuibeng%2==0){
	       Shuibeng(ON);
				 cgq_proper.shuibengzhuangtai=TRUE;
		  }
			else if(shuibeng%2==1){
			   Shuibeng(OFF);
				 cgq_proper.shuibengzhuangtai=FALSE;
				
			}
			++shuibeng;
	}
	if(strstr((const char*)buf,"ACB")!=NULL){ //蜂鸣器
	    if(fengming%2==0){
	       Fengming(ON);
				 cgq_proper.Buzzer=TRUE;
		  }
			else if(fengming%2==1){
			   Fengming(OFF);
				 cgq_proper.Buzzer=FALSE;
				
			}
			++fengming;
	}
	
	
	
}


/*
 * 功能：读取各传感器的数值，将数值读取到CGQ_Buffer中；
 * 参数：void
 * 返回值：char数组
*/
char*  read_cgq(void){
  
	cgq_proper.Env_lux=(double)Read_lux();
	cgq_proper.soilHumidity=(double)Read_soil();
	
	sprintf(TxBuffer1,"{\"id\":\"123\",\"iotId\":\"JjvYMZ7j2ZGenDj3sZWU000100\", \
	\"method\":\"thing.event.property.post\",\"params\":{\"Buzzer\":%d,\"chuangliankaiguan\":%d,\
	\"CombustibleGasCheck\":%g,\"Env_lux\":%g,\"LightSwitch\":%d,\
	\"shuibengzhuangtai\":%d,\"soilHumidity\":%g,\"Temperature\":%g},\
	\"topic\":\"/sys/a1xFHTXX9bA/stm32/thing/event/property/post\",\"uniMsgId\":\"5081150873750638592\",\
	\"version\":\"1.0\" }",cgq_proper.Buzzer,cgq_proper.chuangliankaiguan,cgq_proper.CombustibleGasCheck,\
	cgq_proper.Env_lux,cgq_proper.LightSwitch,cgq_proper.shuibengzhuangtai,\
	cgq_proper.soilHumidity,cgq_proper.Temperature);
	
	return TxBuffer1;

}
/*处理云平台payload，并控制执行器*/
void processServer(unsigned char*payload_IN){

	
	if(strstr((const char*)payload_IN,"\"shuibengzhuangtai\":1")!=NULL ||
		strstr((const char*)payload_IN,"\"sb_switch\":1")!=NULL){
	   Shuibeng(ON);
		 cgq_proper.shuibengzhuangtai=TRUE;
	}
	else if(strstr((const char*)payload_IN,"\"shuibengzhuangtai\":0")!=NULL || 
		  strstr((const char*)payload_IN,"\"sb_switch\":0")!=NULL){
	   Shuibeng(OFF);
		 cgq_proper.shuibengzhuangtai=FALSE;
	}
	
	if(strstr((const char*)payload_IN,"\"Buzzer\":1")!=NULL || 
		strstr((const char*)payload_IN,"\"buzzer_switch\":1")!=NULL){
	   Fengming(ON);
		 cgq_proper.Buzzer=TRUE;
	}
	else if(strstr((const char*)payload_IN,"\"Buzzer\":0")!=NULL || 
		strstr((const char*)payload_IN,"\"buzzer_switch\":0")!=NULL){
	   Fengming(OFF);
		 cgq_proper.Buzzer=FALSE;
	}
	
	if(strstr((const char*)payload_IN,"\"LightSwitch\":1")!=NULL || 
		strstr((const char*)payload_IN,"\"LED_switch\":1")!=NULL){
	   LED(ON);
		 cgq_proper.LightSwitch=TRUE;
	}
	else if(strstr((const char*)payload_IN,"\"LightSwitch\":0")!=NULL|| 
		strstr((const char*)payload_IN,"\"LED_switch\":0")!=NULL){
	   LED(OFF);
		 cgq_proper.LightSwitch=FALSE;
	}
	
}


