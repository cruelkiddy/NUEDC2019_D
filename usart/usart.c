#include "sys.h"
#include "usart.h"	  
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��ucos,����������ͷ�ļ�����.

extern StateStatus CurrentState;
extern u16 ReceivedVpp;

#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos ʹ��	  
#include "StateMachine.h"
#include "ad5260.h"

#endif
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//����1��ʼ��		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/8/18
//�汾��V1.5
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved
//********************************************************************************
//V1.3�޸�˵�� 
//֧����Ӧ��ͬƵ���µĴ��ڲ���������.
//�����˶�printf��֧��
//�����˴��ڽ��������.
//������printf��һ���ַ���ʧ��bug
//V1.4�޸�˵��
//1,�޸Ĵ��ڳ�ʼ��IO��bug
//2,�޸���USART_RX_STA,ʹ�ô����������ֽ���Ϊ2��14�η�
//3,������USART_REC_LEN,���ڶ��崮�����������յ��ֽ���(������2��14�η�)
//4,�޸���EN_USART1_RX��ʹ�ܷ�ʽ
//V1.5�޸�˵��
//1,�����˶�UCOSII��֧��
////////////////////////////////////////////////////////////////////////////////// 	  
 

//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

/*ʹ��microLib�ķ���*/
 /* 
int fputc(int ch, FILE *f)
{
	USART_SendData(USART1, (uint8_t) ch);

	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}	
   
    return ch;
}
int GetKey (void)  { 

    while (!(USART1->SR & USART_FLAG_RXNE));

    return ((int)(USART1->DR & 0x1FF));
}
*/
 
#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA=0;       //����״̬���	 
u16 singal_flag = 0;			//����״̬���
u8 change_appare = 0;		//�����Ƿ���±�ǣ�����δ����ϢΪ1������Ϊ0
u8 is_changing = 0;			//�����Ƿ����ڸ��£����ڸ�������Ϊ1������Ϊ0

u16 use = 0;				  //��Ч���ݵĸ���

u8 in_key = 'a';			//�Զ����ͷ��Ҫ���ͷ���ִ���Ϊ2
u8 out_key = 'b';		//�Զ����β��Ҫ���β���ִ���Ϊ2

u8 my_data[my_len];			//�Զ���������� 
  
void uart_init(u32 bound){
  //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.9
   
  //USART1_RX	  GPIOA.10��ʼ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.10  

  //Usart1 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

  USART_Init(USART1, &USART_InitStructure); //��ʼ������1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
  USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���1 

}

void USART1_IRQHandler(void)                	//����1�жϷ������
	{
	int i;
	u8 Res;
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif
		/*if(USART_GetITStatus(USART1,USART_IT_RXNE) != RESET )
		{
			Res = USART_ReceiveData(USART1);
			USART_RX_BUF[0] = Res;			
		}*/
	
	if(USART_GetITStatus(USART1,USART_IT_RXNE) != RESET )
	{
		Res = USART_ReceiveData(USART1);
		
		if((singal_flag&0X8000) == 0) //δ����¼��ѭ��
		{
			if(Res == in_key){
				singal_flag = 0X8000;			
			}
		}
		else if((singal_flag&0X4000) == 0)	//ֻ����˵�һ���ж�
		{
			if(Res == in_key){
				singal_flag |= 0X4000;
				is_changing = 1;
				
				//��������������룬�����ʱdata�����ݻ�δ������ʾ�Ļ��������ݲ��������ݶ��˳�ѭ��
				/*****
				if(change_appare == 1)
					singal_flag = 0;
				*****/
			}

			else
				singal_flag = 0;
		}
		else if((singal_flag&0X2000) == 0)	//����¼��״̬
		{
			
			if((singal_flag&0X0FFF) > my_len - 1){
				singal_flag = 0;
				is_changing = 0;
				for(i = 0; i < my_len; i++){
					my_data[i] = 0;
				}
			}
			else{
				if(Res == out_key)
					singal_flag |= 0X2000;
				my_data[(singal_flag&0X0FFF)] = Res;
				singal_flag ++;
			}
		}
		else		//������Ƴ�״̬
		{
			if(Res == out_key){
				use = singal_flag & 0X0FFF;
				
				singal_flag = 0;
				is_changing = 0;
				change_appare = 1;
				
			}
			else{
				
				singal_flag &= 0XDFFF;
				
				if((singal_flag&0X0FFF) > my_len - 1){
					singal_flag = 0;
					is_changing = 0;
					for(i = 0; i < my_len; i++){
						my_data[i] = 0;
					}
				}
				else{
					my_data[(singal_flag&0X0FFF)] = Res;
					singal_flag ++;
				}
			}
		}
		if(change_appare == 1){
						change_appare = 0;
						switch(my_data[0]){
							case '1': CurrentState = MeasureRi;  break;
							case '2': CurrentState = MeasureRo;  break;
							case '3': CurrentState = MeasureAu;  break;				///< TODO
							case '4': CurrentState = MeasureSweep; break;     ///< TODO
							case '5': CurrentState = DebugCircuit;  break;     ///< TODO
							case '9':  break;			///< Used as a switch
						}				
		}		
	}
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif
} 
#endif	

