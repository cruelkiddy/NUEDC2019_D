#include "StateMachine.h"
#include "stm32f10x_adc.h"  ///< To get ADC address
#include "ad5260.h"
#include "timer.h"
#include "delay.h"
#include "CONTROL.h"
#include "gpio.h"

extern StateStatus CurrentState;
u16 TestBuffer[10];
/**
 * 			  9:05 			8/7/2019		屏蔽中断，蒙住眼睛
 *				11:27     8/8/2019		状态机的架构和通信的接收已经完成
 *                          TODO自适应ADC采样速率变换 
 * 				
 *        PIN Configuration:  PA1   ADC1    Vout
 *														PA2   ADC3    Vin
 *                            PE2   GPIO    Interrupt
 *                            PB6   GPIO    Switch for Load
 *                            PA9   UART1
 *                            PA10  UART1   
 *														PG3   				SWITCHA
 *                            PG4   				SWITCHC
 *                            PG13          AmpSwitchB		BC 10 On 01 Off
 *                            PG15          AmpSwitchC
 *														PB12  AD5260_CS
 *                            PB14  AD5260_CLK
 *                            PB15  SDI
 *                            PB5   SquareWave 100Hz
 *                            PG1   SourceSelect  0->Square  1->DDS
 *
 *
 *        9:14      8/9/2019  比较稳定的基础部分框架
 *                            TODO发挥部分
 *                            SerialWrite ISSUE
 *                            
 *             Switch   PBout(6)		测量Ro用到的模拟开关
 *					   Switch_B PBout(7)    暂时未启用
 *						 Switch_A PGout(3)    控制数字电位器 
 *             Switch_C PGout(4)    控制数字电位器 
 *             修改了ad5260的参数表
 *             修改了测量Vpp的函数
 *             改后未测	StateMeasureAu
 */

u16 TestIndex = 0;

 int main(void){  	 
		delay_init();
	  uart_init(115200);
		
	 
		LCD_Init();
	  
	  InSwitch_Init();		///< PG3 PG4  In Switch    A  C
    //SwitchB_Init();
	  AmpSwitch_Init();
	  LoadSwitch_Init();
	  AD5260_Init();
	  PGONEInit();
	  ///< Testing!!!!!!
	  TIM3_PWM_Init(199, 7200 - 1);			///< Generate 50Hz Square Wave
    TIM_SetCompare2(TIM3, 100);
		
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	  GPIO_EXTI_Init();		///< PE2
	  
	  Switch_A = 1;				//A = 0; C = 1可变电阻
	  Switch_C = 0;
	  Switch = 0;					//空载
	  AmpSwitchB = 0;			//输入监测点不放大
	  AmpSwitchC = 1;
		SourceSelect = 1;		//DDS输入
		
		for(;;){
					switch(CurrentState){
						case MeasureRi: StateMeasureRi(); break;
						case MeasureRo: StateMeasureRo(); break;						///< TODO
						case MeasureAu: StateMeasureAu(); break;
						case MeasureSweep: StateMeasureSweep(); break;
						case DebugCircuit: StateDebugCircuit(); break;														///< TODO
						case IDLE: break;
					}
		}
		
}	


 
 