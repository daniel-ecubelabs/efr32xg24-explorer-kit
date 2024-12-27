/***************************************************************************//**
 * @file
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include <stdio.h>
#include "sl_uartdrv_instances.h"
#include "sl_uartdrv_eusart_mikroe_config.h"

int _write(int file, char *ptr, int len)
{
    UARTDRV_TransmitB(sl_uartdrv_get_default(), (uint8_t*)ptr, len);
    return len;
}
/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  static char str[16] = "Hello World\r\n";
   UARTDRV_Transmit(sl_uartdrv_get_default(), (uint8_t*)str, 16, NULL);

   printf("app_init++\r\n");

   UARTDRV_Transmit(sl_uartdrv_eusart_mikroe_handle, (uint8_t*)str, 16, NULL);

  NVIC_ClearPendingIRQ(EUSART1_RX_IRQn);
  NVIC_EnableIRQ(EUSART1_RX_IRQn);

  EUSART_IntEnable(EUSART1, EUSART_IEN_RXFL);

//  __enable_irq();

  UARTDRV_Receive(sl_uartdrv_eusart_mikroe_handle, (uint8_t*)str, 1, NULL);

}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

void EUSART1_RX_IRQHandler(void)
{
  uint8_t ch = 0;

#if 0//레지스터 활성화 코드 삽입 필요

  if (EUSART1->IF & EUSART_IF_RXFL) {
      ch = (uint8_t)(EUSART1->RXDATA);
      // Process the received data
  }
#else
  UARTDRV_Receive(sl_uartdrv_eusart_mikroe_handle, &ch, 1, NULL);
#endif
  /*
   * The EUSART differs from the USART in that explicit clearing of
   * RX interrupt flags is required even after emptying the RX FIFO.
   */
  EUSART_IntClear(EUSART1, EUSART_IF_RXFL);

}
