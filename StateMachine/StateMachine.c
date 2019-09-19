#include "StateMachine.h"
#include "stm32f10x_adc.h"
#include "Control.h"
#include "usart.h"
#include "ad5260.h"
#include "lcd.h"
#include "gpio.h"
#include "table.h"

extern u16 RiLookUpTable[1601];

u8 Data = 128;

///< Global Var
StateStatus CurrentState = IDLE;		///< Initialization!!!!!
u16 DAC_Buffer[BufferSize];
u16 ADC3_Buffer[BufferSize];
u16 GlobalIndex;
u8 UartBuffer[5];
u16 ReceivedVpp;

extern u8 my_data[my_len];

///< Calc Parameters
u32 Rt = 1000;
u32 Rl1 = 100;
u32 Rl2 = 100;


/// R = (256 - Data)  /  256  *  20k   +   60
void StateMeasureRi(void){
	  u16 Vmin = 4095;
	  u16 Vmax = 0;
	  u32 Vavg = 0;
	  u16 Index = 0;
	  u16 MeasureCounter = 0;
	  u32 Ri = 0;
		Adc3_Init();
    MYDMA_Config_2(DMA2_Channel5, (u32)&ADC3->DR, (u32)ADC3_Buffer, BufferSize);
	  
	  if(my_data[1] == '1')	{Switch_A = 1;Switch_C = 0;}
		else if(my_data[1] == '2'){Switch_A = 1;Switch_C = 0;}
		else if(my_data[1] == '3'){Switch_A = 0;Switch_C = 1;}
		/*
		for(MeasureCounter = 0; MeasureCounter < 10; MeasureCounter ++){
			  Vmin = 4095;Vmax = 0;
				MYDMA_Enable_2(DMA2_Channel5);
			  while(DMA_GetFlagStatus(DMA2_FLAG_TC5)==RESET);
			  DMA_ClearFlag(DMA2_FLAG_TC5);
			  for(Index = 0; Index < BufferSize; Index ++){
						if(Vmax < ADC3_Buffer[Index])  Vmax = ADC3_Buffer[Index];
						if(Vmin > ADC3_Buffer[Index])  Vmin = ADC3_Buffer[Index];
				}
				Vavg += (Vmax - Vmin);
				
		}
		Vavg = Vavg / 10;
    */
		Vavg = getAverageVpp(3, 1);
		
		if(my_data[1] == '1')	{
				//Ri = (int)(Vavg*1000.0/(3289 - Vavg));
			  //Ri = (Vavg*3300.0/4095)/(2.4189 - (Vavg*3.3/4095));
			  //Ri = Vavg;
			  //printf("%d\r\n", Vavg);
				if(Vavg <= 2955 && Vavg >= 1500)
						Ri = RiLookUpTable[Vavg-1500];
				else if(Vavg < 1500) Ri = 1000;
				else	               Ri = 50000;
				
				if(Ri <= 2790 && Ri >= 2710)
						Ri = 2100 + Vavg * 0.01;
				
				LCD_ShowString(20, 30, 200, 30, 16, "Ri_1_Ri_Vavg");
				LCD_ShowNum(10, 50, Ri, 16, 16);
			  LCD_ShowNum(10, 80, Vavg, 16, 16);
				SendHead();
				USART_SendData(USART1, (Ri&0xff000000) >> 24);//向串口1发送数据
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
				USART_SendData(USART1, (Ri&0x00ff0000) >> 16);//向串口1发送数据
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
				USART_SendData(USART1, (Ri&0x0000ff00) >> 8);//向串口1发送数据
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
				USART_SendData(USART1, Ri&0xff);//向串口1发送数据
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
				SendTail();
		}	
		
		
		else if(my_data[1] == '2')	{
			  //Ri = (int)(Vavg*1000.0/(3289 - Vavg));
			  Ri = 2154;		///< 
				LCD_ShowString(20, 30, 200, 30, 16, "Ri_2_Test");
				SendHead();
				USART_SendData(USART1, (Ri&0xff000000) >> 24);//向串口1发送数据
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
				USART_SendData(USART1, (Ri&0x00ff0000) >> 16);//向串口1发送数据
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
				USART_SendData(USART1, (Ri&0x0000ff00) >> 8);//向串口1发送数据
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
				USART_SendData(USART1, Ri&0xff);//向串口1发送数据
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
				SendTail();
		}	
		
		
		else if(my_data[1] == '3')	{

///< Tring to auto adc scale	
        TurnOnAmp();	
        AD5260_SEND(220);	///< 3K Ohm
			  delay_ms(10);
				/*
				while(1){
						Vmin = 4095;Vmax = 0;
						MYDMA_Enable_2(DMA2_Channel5);
						while(DMA_GetFlagStatus(DMA2_FLAG_TC5)==RESET);
						DMA_ClearFlag(DMA2_FLAG_TC5);
						for(Index = 0; Index < BufferSize; Index ++) {
								if(Vmax < ADC3_Buffer[Index])  Vmax = ADC3_Buffer[Index];
								if(Vmin > ADC3_Buffer[Index])  Vmin = ADC3_Buffer[Index];
						}
						LCD_ShowNum(50, 50, Vmax - Vmin, 16, 16);
						
						AD5260_SEND(Data);delay_ms(10);
						//Data += 5;

						LCD_ShowNum(20, 20, Data, 16, 16);
						//if(Data == 255)  {Data = 0;break;}
										
						if((Vmax - Vmin) > 260)	{
								Data-=1;
								AD5260_SEND(Data); 
							  delay_ms(1);
						}
						else if((Vmax - Vmin) < 254){
								Data += 1;
							  AD5260_SEND(Data);
							  delay_ms(1);
						}
						else break;	
				}
				*/
				Vavg = getAverageVpp(3, 3);
				if(Vavg < 372.0/3) {
					 AD5260_SEND(141);		///< 9K Ohm
					 delay_ms(10);
					 Vavg = getAverageVpp(3, 3);
					 Ri = (int)9000.0 * Vavg / (372-Vavg);///< Ohm
				}
			  else if(Vavg > 8.0/9*372){
					 AD5260_SEND(218);	
					 delay_ms(10);
				   Vavg = getAverageVpp(3, 3);
					 Ri = (int)3000.0 * Vavg / (372-Vavg);	///< Ohm
				}
				else
					 Ri = (int)3000.0 * Vavg / (372-Vavg);	///< Ohm
				
				
				LCD_ShowString(20, 30, 200, 30, 16, "Ri_3_Ri_Vavg");
				LCD_ShowNum(10, 50, Ri, 16, 16);
			  LCD_ShowNum(10, 80, Vavg, 16, 16);
				
				SendHead();
				USART_SendData(USART1, (Ri&0xff000000) >> 24);//向串口1发送数据
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
				USART_SendData(USART1, (Ri&0x00ff0000) >> 16);//向串口1发送数据
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
				USART_SendData(USART1, (Ri&0x0000ff00) >> 8);//向串口1发送数据
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
				USART_SendData(USART1, Ri&0xff);//向串口1发送数据
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
				SendTail();		
				
				Switch_A = 1; Switch_C = 0;
				TurnOffAmp();
			}

    
		CurrentState = IDLE;
}

///< Think Carefully Here
void StateMeasureSweep(void){
		Switch = 0;		///< Kongzai
		Adc_Flex_Init(ADC_SampleTime_239Cycles5);
		MYDMA_Config(DMA1_Channel1, (u32)&ADC1->DR, (u32)DAC_Buffer, BufferSize);
		for(;;){
				if(CurrentState != MeasureSweep) return;       ///< Exit if State is wrong
				else{																					 ///< Do something if State is right
																											 ///< Do nothing, wait for interrupt 
				}
	 }
}

void StateMeasureAu(void){
    u16 Vmin = 4095;
	  u16 Vmax = 0;
	  u16 Vpp_In = 0;
	  u16 Vpp_Out = 0;
	  u16 Index = 0;
	  u32 Au100x = 0;
	  
		Switch = 0;		   ///< 空载
		AmpSwitchB = 0;  ///< 不放大
	  AmpSwitchC = 1;  
		
		Adc3_Init();
    MYDMA_Config_2(DMA2_Channel5, (u32)&ADC3->DR, (u32)ADC3_Buffer, BufferSize);
	
		Adc_Flex_Init(ADC_SampleTime_239Cycles5);
		MYDMA_Config(DMA1_Channel1, (u32)&ADC1->DR, (u32)DAC_Buffer, BufferSize);
	  
	  /*
		MYDMA_Enable_2(DMA2_Channel5);
	  
		for(;;){
				if(CurrentState != MeasureAu)		return;
				else{
						if(DMA_GetFlagStatus(DMA2_FLAG_TC5)!=RESET) {
							DMA_ClearFlag(DMA2_FLAG_TC5);
						  for(Index = 0; Index < BufferSize; Index ++){
								if(Vmax < ADC3_Buffer[Index])  Vmax = ADC3_Buffer[Index];
								if(Vmin > ADC3_Buffer[Index])  Vmin = ADC3_Buffer[Index];
							}
							///< Calculate Here
							///< Send Here
		*/
					if(my_data[1] == '1'){
							Vpp_Out = getAverageVpp(1, 10);
							Vpp_In = getAverageVpp(3, 10);
							
							//Au100x = Vpp_Out * 100 / Vpp_In;
							Au100x = (int)((527.27*Vpp_Out*3300.0/4095) / 30);
						
						  LCD_ShowString(20, 30, 200, 30, 16, "Au_1_Out_In");
				      LCD_ShowNum(10, 50, Vpp_Out, 16, 16);
			        LCD_ShowNum(10, 80, Vpp_In, 16, 16);
							
							//Au100x = (int)((100*5.2727*Vpp*3300.0/4095) / 30);
							SendHead();
							//SerialWrite((u8*)Au100x, 4);
							USART_SendData(USART1, (Au100x&0xff000000) >> 24);//向串口1发送数据
							while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
							USART_SendData(USART1, (Au100x&0x00ff0000) >> 16);//向串口1发送数据
							while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
							USART_SendData(USART1, (Au100x&0x0000ff00) >> 8);//向串口1发送数据
							while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
							USART_SendData(USART1, Au100x&0xff);//向串口1发送数据
							while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
							SendTail();
					
							CurrentState = IDLE;		///< Change State to IDLE
					}
					else if(my_data[1] == '2'){
							Au100x = 135*100;
						  LCD_ShowString(20, 30, 200, 30, 16, "Au_2_Test");
							SendHead();
							//SerialWrite((u8*)Au100x, 4);
							USART_SendData(USART1, (Au100x&0xff000000) >> 24);//向串口1发送数据
							while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
							USART_SendData(USART1, (Au100x&0x00ff0000) >> 16);//向串口1发送数据
							while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
							USART_SendData(USART1, (Au100x&0x0000ff00) >> 8);//向串口1发送数据
							while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
							USART_SendData(USART1, Au100x&0xff);//向串口1发送数据
							while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
							SendTail();
					
							CurrentState = IDLE;		///< Change State to IDLE							
					}
	}		
	
void	StateMeasureRo(void){
		u16 V1 = 0;			///< 悠哉的输出电压
	  u16 V2 = 0;			///< 空载的输出电压
		u16 Index = 0;
	  u16 Vmax = 0;
	  u16 Vmin = 4095;
	  u16 Ro = 0;
		Adc_Flex_Init(ADC_SampleTime_239Cycles5);
		MYDMA_Config(DMA1_Channel1, (u32)&ADC1->DR, (u32)DAC_Buffer, BufferSize);
		
		Switch = 1;		///< 带负载
		
		DelaySomeTime();
	  //MYDMA_Enable(DMA1_Channel1);
	  for(;;){
				if(CurrentState != MeasureRo)		return;
				
				else{
/*
					if(DMA_GetFlagStatus(DMA1_FLAG_TC1)!=RESET) {
							DMA_ClearFlag(DMA1_FLAG_TC1);
						  for(Index = 0; Index < BufferSize; Index ++){
								if(Vmax < DAC_Buffer[Index])  Vmax = DAC_Buffer[Index];
								if(Vmin > DAC_Buffer[Index])  Vmin = DAC_Buffer[Index];
							}
				  V1 = Vmax - Vmin;
					Vmax = 0;
	        Vmin = 4095;
					Switch = 0;		///<去负载
					
					DelaySomeTime();
					MYDMA_Enable(DMA1_Channel1);
					while(DMA_GetFlagStatus(DMA1_FLAG_TC1)==RESET);
					
					DMA_ClearFlag(DMA1_FLAG_TC1);
					for(Index = 0; Index < BufferSize; Index ++){
					   	if(Vmax < DAC_Buffer[Index])  Vmax = DAC_Buffer[Index];
							if(Vmin > DAC_Buffer[Index])  Vmin = DAC_Buffer[Index];
					}
					
					V2 = Vmax - Vmin;
					Ro = (int)(V2 - V1)*5100.0 / V1;
					
					SendHead();
					//SerialWrite((u8*)&Ro, 2);
					USART_SendData(USART1, Ro&0xff00 >> 8);//向串口1发送数据
				  while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
					USART_SendData(USART1, Ro&0x00ff);//向串口1发送数据
				  while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
					SendTail();
						
					Switch = 0;
					
					CurrentState = IDLE;
					
				}
*/				if(my_data[1] == '1'){
					V1 = getAverageVpp(1, 10);
					Switch = 0;												///<去负载
					delay_ms(10);
					V2 = getAverageVpp(1, 10);
				
				  Ro = (int)((V2 - V1)*5100.0 / V1);	///< 最多测到3K Ohm
					LCD_ShowString(20, 30, 200, 30, 16, "Ro_1_Ro_Vpp");
			    LCD_ShowNum(10, 50, Ro, 16, 16);
				  LCD_ShowNum(10, 80, V2 - V1, 16, 16);
	
          
					SendHead();
					USART_SendData(USART1, (Ro&0xff00) >> 8);//向串口1发送数据
				  while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
					USART_SendData(USART1, Ro&0x00ff);//向串口1发送数据
				  while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
					SendTail();
					
					Switch = 0;
				  CurrentState = IDLE;		
					}
					else if(my_data[1] == '2'){
							Ro = 1995;
						  LCD_ShowString(20, 30, 200, 30, 16, "Ro_2_Test");
			        LCD_ShowNum(10, 50, Ro, 16, 16);						  
						  SendHead();
							USART_SendData(USART1, (Ro&0xff00) >> 8);//向串口1发送数据
							while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
							USART_SendData(USART1, Ro&0x00ff);//向串口1发送数据
							while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
							SendTail();
						  CurrentState = IDLE;
					}
		}
  }
}

///< ADC1

void	StateDebugCircuit(void){
		Switch = 1;	///< Begin with Load!!!
	  for(;;)	{
			if(CurrentState != DebugCircuit)  return;
			else{
				Adc_Flex_Init(ADC_SampleTime_239Cycles5);		///< Get ready for 1kHz, 30mVpp
				MYDMA_Config(DMA1_Channel1, (u32)&ADC1->DR, (u32)DAC_Buffer, BufferSize);
		
				Adc3_Flex_Init(ADC_SampleTime_239Cycles5);
				MYDMA_Config_2(DMA2_Channel5, (u32)&ADC3->DR, (u32)ADC3_Buffer, BufferSize);

				DelaySomeTime();
				
				MYDMA_Enable(DMA1_Channel1);
				MYDMA_Enable_2(DMA2_Channel5);
				while(DMA_GetFlagStatus(DMA2_FLAG_TC5)==RESET);
				DMA_ClearFlag(DMA2_FLAG_TC5);
				while(DMA_GetFlagStatus(DMA1_FLAG_TC1)==RESET);
				DMA_ClearFlag(DMA1_FLAG_TC1);
				
				DC_flag = DC_AC(getVmax(DAC_Buffer, BufferSize), getVmin(DAC_Buffer, BufferSize));
				//DC_AC_IN(getVmax(ADC3_Buffer, BufferSize), getVmin(ADC3_Buffer, BufferSize));
				LCD_ShowString(10, 100, 200, 16, 16, "Debug_Judge1");
				LCD_ShowNum(10, 120, DC_flag, 4, 16);
				LCD_ShowString(10, 140, 200, 16, 16, "AVG_Vpp");
				LCD_ShowNum(10, 160, avg, 20, 16);
				LCD_ShowNum(10, 180, vpp, 20, 16);
				JUDGE_1(avg, vpp);
				CurrentState = IDLE;
				Switch = 0;
			}
	  }
}

void DelaySomeTime(void){
		u16 Counter = 0;
		for(;Counter < 60000;Counter++);
}


///< Todo  OPTIMIZE
u16 getVmin(u16* Array, u16 Size){
	  u16 Vmin = 4095;	
    u16 Index;	
		for(Index = 0; Index < Size; Index ++){
				if(Vmin > Array[Index])  Vmin = Array[Index];
		}
		return Vmin;
}		

///< Todo  OPTIMIZE
u16 getVmax(u16* Array, u16 Size){
	  u16 Vmax = 0;	
    u16 Index;	
		for(Index = 0; Index < Size; Index ++){
				if(Vmax < Array[Index])  Vmax = Array[Index];
		}
		return Vmax;		
}		

u16 getAverageVpp(u8 adc, u8 Count) {
	  u16 MeasureCounter = 0;
	  u16 Vmin = 4095;
	  u16 Vmax = 0;
	  u16 Index = 0;
	  u32 Vavg = 0;
		if(adc == 3){
     		for(MeasureCounter = 0; MeasureCounter < Count; MeasureCounter ++){
			  Vmin = 4095;Vmax = 0;
				MYDMA_Enable_2(DMA2_Channel5);
			  while(DMA_GetFlagStatus(DMA2_FLAG_TC5)==RESET);
			  DMA_ClearFlag(DMA2_FLAG_TC5);
			  for(Index = 0; Index < BufferSize; Index ++){
						if(Vmax < ADC3_Buffer[Index])  Vmax = ADC3_Buffer[Index];
						if(Vmin > ADC3_Buffer[Index])  Vmin = ADC3_Buffer[Index];
				}
				Vavg += (Vmax - Vmin);
				
		}
		return Vavg / Count;
			
		}
		else if(adc == 1){
     		for(MeasureCounter = 0; MeasureCounter < Count; MeasureCounter ++){
			  Vmin = 4095;Vmax = 0;
				MYDMA_Enable(DMA1_Channel1);
		    while(DMA_GetFlagStatus(DMA1_FLAG_TC1)==RESET);
		    DMA_ClearFlag(DMA1_FLAG_TC1);
			  for(Index = 0; Index < BufferSize; Index ++){
						if(Vmax < DAC_Buffer[Index])  Vmax = DAC_Buffer[Index];
						if(Vmin > DAC_Buffer[Index])  Vmin = DAC_Buffer[Index];
				}
				Vavg += (Vmax - Vmin);
				
		}
		return Vavg / Count;		
		
		}
}
