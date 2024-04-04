#include "debug.h"
#include <ch32v00x.h>

// Based on the samples in https://www.wch.cn/downloads/CH32V003EVT_ZIP.html.

// Configure all pins on peripheral A and C to input pulldown to save maximum
// power.
void SetupOtherPins() {
  GPIO_InitTypeDef GPIO_InitStructure = {0};

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;

  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_Init(GPIOC, &GPIO_InitStructure);
}

// Initialize pin D0 as external interrupt and pull all pins in D up for power
// saving.
void InitializeInterrupt() {
  GPIO_InitTypeDef GPIO_InitStructure = {0};
  EXTI_InitTypeDef EXTI_InitStructure = {0};
  NVIC_InitTypeDef NVIC_InitStructure = {0};

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOD, ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  /* GPIOD.0 ----> EXTI_Line0 */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource0);
  EXTI_InitStructure.EXTI_Line = EXTI_Line0;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = EXTI7_0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

int main(void) {
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  SystemCoreClockUpdate();

  Delay_Init();
  Delay_Ms(1000);
  Delay_Ms(1000);

  InitializeInterrupt();
  SetupOtherPins();

  USART_Printf_Init(115200);

  printf("Standby Mode External Interrupt Test\r\n");
  printf("\r\n ********** \r\n");

  RCC_LSICmd(ENABLE);
  while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET) {
  };
  PWR_AWU_SetPrescaler(PWR_AWU_Prescaler_10240);
  PWR_EnterSTANDBYMode(PWR_STANDBYEntry_WFE);

  USART_Printf_Init(115200);
  printf("\r\n ########## \r\n");

  while (1) {
    Delay_Ms(1000);
    printf("Run in main\r\n");
  }
}

extern "C" void EXTI7_0_IRQHandler(void)
    __attribute__((interrupt("WCH-Interrupt-fast")));

extern "C" void EXTI7_0_IRQHandler(void) {
  if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
    printf("EXTI0 Wake_up\r\n");
    EXTI_ClearITPendingBit(EXTI_Line0); /* Clear Flag */
  }
}
