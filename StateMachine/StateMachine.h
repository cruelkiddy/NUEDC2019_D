#ifndef __STATEMACHINE_H
#define __STATEMACHINE_H	

#include "delay.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"	 
#include "adc.h"
#include "math.h"
#include "dma.h"
#include "timer.h"
#include "MY_UART.h"
#include "usart.h"
#include "timer.h"
#include "gpio.h"
#include "led.h"


typedef enum{
		MeasureRi = 0,
		MeasureRo,
		MeasureAu,
		MeasureSweep,
		DebugCircuit,
	  IDLE
}StateStatus;

#define BufferSize 256

void StateMeasureRi(void);
void StateMeasureSweep(void);
void StateMeasureAu(void);
void StateMeasureRo(void);
void	StateDebugCircuit(void);
void DelaySomeTime(void);

u16 getVmin(u16* Array, u16 Size);
u16 getVmax(u16* Array, u16 Size);

u16 getAverageVpp(u8 adc, u8 Count);
u16 getVpp(u8 adc);

/// Global Var for debugging
extern u16 max;     //for test
extern u16 min;        //for test 
extern u16 max_in;  //for test
extern u16 min_in;     //for test 

extern u16 avg;  //UOUT��ʱ��ƽ��ֵ
extern u16 vpp;//UOUT���ֵ
extern u16 avg_in;  // UIN��ʱ�����ƽ��ֵ
extern u16 vpp_in;//UIN������ֵ
extern u8 DC_flag;  //�ж��Ƿ�ΪDC Ϊ1��DC

#endif 
