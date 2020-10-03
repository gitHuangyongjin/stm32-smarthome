
/**************************************
 * 文件名  ：main.c   
 * 实验平台：MINI STM32开发板 基于STM32F103C8T6
 * 库版本  ：ST3.0.0  																										  

*********************************************************/

#include "stm32f10x.h"
#include <string.h>
#include <stdio.h>
#include "MQTTPacket.h"
#include "usart1.h"
#include "ESP8266.h"
#include "SysTick.h"
#include "adc.h"
#include "dma.h"
#include "led.h"
#include "exti.h"


#define SSD "****"
#define PWD "****"

#ifndef  MQTT_MAX_BUF_SIZE
#define MQTT_MAX_BUF_SIZE 500
#endif

#define Near_IP1 "192.168.2.105"
#define Near_IP2 "192.168.2.114"
#define Near_port 8080

#ifndef MODE
#define MODE "TCP"
#define IP "a1xFa1xFHTXX9bA.iot-as-mqtt.cn-shanghai.aliyuncs.com"
#define PORT 1883


#endif

#define MQTT_SERVER_IP_AND_PORT "a1xFa1xFHTXX9bA.iot-as-mqtt.cn-shanghai.aliyuncs.com, 1883"



int main(void)
{  
	 //mqtt相关参数
	  u16  retry_count = 5;
    unsigned short submsgid;
    unsigned char buf[MQTT_MAX_BUF_SIZE];
    unsigned char sessionPresent, connack_rc;
    const char* payload = "mypayload";
    int payloadlen = strlen(payload);
    int ret = 0,len = 0,req_qos = 0,msgid = 1,granted_qos,subcount;

    MQTTString topicString = MQTTString_initializer;
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;   
	//系统初始化
  SystemInit();	
	USART1_Config(); 
	SysTick_Init();
	Adc_Init();
	DMA_Configuration();
	LED_GPIO_Config();
	EXTI_Configuration();
	initCGQ();
	
	
	if(initESP8266()==1)
			{
		    //提示OLED 
				if(connectAP(SSD,PWD)==1)
			{
		    
				//根据按键的值 循环进入远场近场
		while(1){		
			    
				if(read_key_down()==0){  //初始化的按键值为0
				  
					/////——————远场进入
					 disconnectServer();
					
           while(connectServer(MODE,IP,PORT)!=1){};
						 
					  data.clientID.cstring = "stm326666|securemode=3,signmethod=hmacsha1,timestamp=6666|";
            data.keepAliveInterval = 40;
            data.cleansession = 1;
            data.username.cstring = "stm32&a1xFHTXX9bA";
            data.password.cstring = "591AC4AFDE29C9127A479B1F6502FB37AE5FE1B3";
					  data.MQTTVersion = 4;
            
            len = MQTTSerialize_connect(buf, MQTT_MAX_BUF_SIZE, &data);
					  ret = transport_sendPacketBuffer(MQTT_SERVER_IP_AND_PORT, buf, len);
            if( ret != 1 ){
              continue;
            }
					  Delay_ms(1000);
						
					 /* wait for connack */
           if (MQTTPacket_read(buf, MQTT_MAX_BUF_SIZE, transport_getdata) != CONNACK)
           {
             continue;
           }
           if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, MQTT_MAX_BUF_SIZE) != 1 || connack_rc != 0)
           {
             continue;
           }
				 
				   memset(buf,0,MQTT_MAX_BUF_SIZE);
				 
          topicString.cstring = "/sys/a1xFHTXX9bA/stm32/thing/service/property/set";
          len = MQTTSerialize_subscribe(buf, MQTT_MAX_BUF_SIZE, 0, msgid, 1, &topicString, &req_qos);
           ret = transport_sendPacketBuffer(MQTT_SERVER_IP_AND_PORT, buf, len);
           if( ret != 1 ){
               continue;
            }
            Delay_ms(1000);
						
          if (MQTTPacket_read(buf, MQTT_MAX_BUF_SIZE, transport_getdata) != SUBACK)   /* wait for suback */
           {
             continue;
          }
		
          MQTTDeserialize_suback(&submsgid, 1, &subcount, &granted_qos, buf, MQTT_MAX_BUF_SIZE);
         if (granted_qos != 1)
         {
             continue;
          }
					
          memset(buf,0,MQTT_MAX_BUF_SIZE);

					
					/* loop getting msgs on subscribed topic  and Near_transmision*/
           topicString.cstring = "/sys/a1xFHTXX9bA/stm32/thing/event/property/post";
			      while( read_key_down()==0 ){   
              memset(buf,0,MQTT_MAX_BUF_SIZE);							
              
             /* transport_getdata() has a built-in 1 second timeout,
               your mileage will vary */
             if (MQTTPacket_read(buf, MQTT_MAX_BUF_SIZE, transport_getdata) == PUBLISH)
             {
                 int qos,payloadlen_in;
                 unsigned char dup,retained;
                 unsigned short msgid;
                  unsigned char* payload_in;
                 MQTTString receivedTopic;

                 MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic,
                    &payload_in, &payloadlen_in, buf, MQTT_MAX_BUF_SIZE);
							 
							 //接收到云端设置topic，设置传感值！！
							  processServer(payload_in);
							 
              }
						 							
						 //在发布消息前也要将buf设为空
							memset(buf,0,MQTT_MAX_BUF_SIZE);		
							
							///读进payload和payloadlen就可以发布了
							payload=read_cgq();
							payloadlen=strlen(payload);
             len = MQTTSerialize_publish(buf, MQTT_MAX_BUF_SIZE, 0, 0, 0, 0, topicString, (unsigned char*)payload, payloadlen);

              //重复发送5次，都不成功则退出
							retry_count=2;
                while( retry_count-- > 0 ){
                    ret = transport_sendPacketBuffer(MQTT_SERVER_IP_AND_PORT, buf, len);
             
                    if( ret == 1 ){
                       break;
                     }
                    Delay_ms(100);
                }
                if( !retry_count && ret != 1 ){

                    continue;
                }
						   	// 每次发送，等待3.5s
                Delay_ms(3500);	
								
            }
					
					}//远场退出
				
          disconnectServer();
				
						
			 ///_______近场进入：
						if(read_key_down()==1){
							unsigned char Near_buf[100];
							
								//近场循环	
              while(connectServer(MODE,Near_IP1,Near_port)==0&&
                    connectServer(MODE,Near_IP2,Near_port)==0){};
					    
							while(read_key_down()==1){
					        char string[30];
								  sprintf(string,"TEMP=28.2|RH=%g",Read_soil());
								  sendBuffertoServer(string);
								   Delay_ms(2000);
								   Near_transport_getdata(Near_buf);
								   Near_processServer(Near_buf);
							}
							disconnectServer();
				 }
						
				 
	 }/////近场远场循环；
		
					
 }
}
	
	while(1){
	   //保证不发生硬件中断
	}

}



