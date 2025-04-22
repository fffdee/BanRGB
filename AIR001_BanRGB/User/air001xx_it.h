#ifndef air001xx_it_H
#define air001xx_it_H

#ifdef __cplusplus
 extern "C" {
#endif 


#include "air001xx_hal_gpio.h"




void NMI_Handler(void);
void HardFault_Handler(void);
void SVC_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void EXTI0_1_IRQHandler(void);
void EXTI2_3_IRQHandler(void);
void USART1_IRQHandler(void); // ´®¿Ú1ÖÐ¶Ï

#ifdef __cplusplus
}
#endif


#endif
