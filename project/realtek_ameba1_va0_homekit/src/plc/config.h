#ifndef _CONFIG_H_
#define _CONFIG_H_

//#define STM8Sxxx        0x01

#define NULL            (void *)0

//#if STM8Sxxx
//    #define DATA_SEG			@near
//#else
//    #define DATA_SEG			
//#endif
typedef signed long         int32;
typedef signed short        int16;
typedef signed char         int8;
typedef unsigned long       uint32;
typedef unsigned short      uint16;
typedef unsigned char       uint8;

typedef volatile signed long    vint32;
typedef volatile signed short   vint16;
typedef volatile signed char    vint8;

typedef volatile uint32         vuint32;
typedef volatile unsigned short vuint16;
typedef volatile unsigned char  vuint8;

//typedef unsigned short uint16_t;
//typedef unsigned char uint8_t;

#define  true				1
#define  false				0

//typedef  unsigned short int time_t;


#define MAX_BUFFER_SZ    512
//#define HSI_VALUE ((u32)16000000) /* Value of the Internal oscillator in Hz*/
//#define SYS_CLK						HSI_VALUE

//#define TIM_CLK_FREQUENCY			SYS_CLK
//#define TICK_TIMER_FREQUENCY		((u32)500)

//#define PWM_CLK             ((u32)1000)    /*just for little current*/
//#define PWM_BULK_CLK             ((u32)1000)    /*just for little current*/

//#define	CHN_PLC				0x00 


//#define UART_CHAR_MAX_DELAY   10//10

//#define SWIM2IO	0

#define CURTAIN_STM8S005

#define PWRCTRL_3               0x00
#define PWRCTRL_86              0x00
#define COLOR_DIMMER            0x00
#define DIMMER_LIGHT            0x00
#define DIMMER                  0x00
#define CURTAIN_MOTOR			0x00
#define DOUBLE_CURTAIN			0x00

#define KEY_REG                 0x00

#define DEBUG                   0x00  

#define PWM1_PIN		GPIO_PIN_3
#define PWM1_PORT	    GPIOD
#define PWM2_PIN		GPIO_PIN_4
#define PWM2_PORT	    GPIOD


//#define GPIO_WriteHigh(port,pin) 	(port->ODR |= (uint8)pin)
//#define GPIO_WriteLow(port,pin) 	(port->ODR &= (uint8)(~pin))
//#define GPIO_ReadInputPin(port,pin) (port->IDR & pin)
//#define GPIO_WriteReverse(port,pin) (port->ODR ^= pin)


#define ADDRESS_LEN     0x04

enum
{
    MEASURE_START = 0,MEASURE_INPROGRESS,MEASURE_COMPLETED,MEASURE_FAILED,MEASURE_DISABLE
};

//#define START_CALIBRATION()\
//do{\
//		TIM2->SR1 = 0x00;\
//		TIM2->IER = TIM2_IER_CC1IE | TIM2_IER_CC2IE;\
//		TIM2->CR1 = TIM2_CR1_CEN;\
//}while(0)

#endif




