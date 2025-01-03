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
#include <stdlib.h>
#include <string.h>

#include "sl_simple_button_instances.h"
#include "sl_uartdrv_instances.h"

volatile char *output_ptr = NULL;
/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  uint8_t ch = 0;

  printf("app_init\r\n");

  static char str[16] = "Hello World\r\n";
  UARTDRV_Transmit(sl_uartdrv_usart_mikroe_handle, (uint8_t*)str, 16, NULL);

  // Enable NVIC USART sources
  NVIC_ClearPendingIRQ(USART0_RX_IRQn);
  NVIC_EnableIRQ(USART0_RX_IRQn);
//
  NVIC_ClearPendingIRQ(USART0_TX_IRQn);
  NVIC_EnableIRQ(USART0_TX_IRQn);
//
  USART_IntEnable(USART0, USART_IEN_RXDATAV);

  //인터럽트 활성화를 우해 1회 호출해 주어야 함.
  UARTDRV_Receive(sl_uartdrv_usart_mikroe_handle, &ch, 1, NULL);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

/**************************************************************************//**
 * @brief
 *    The USART0 receive interrupt saves incoming characters.
 *    This function removes \r and \n characters.
 *    Calls global callback if it is defined.
 *    Uses its own buffer to store.
 *    Do not block in this function!
 *
 *****************************************************************************/
void USART0_RX_IRQHandler(void)
{
  uint8_t data;

  data = USART_Rx(USART0);

  printf("%c", data);

  return;
}

/**************************************************************************//**
 * @brief
 *    The USART0 transmit interrupt outputs characters.
 *****************************************************************************/
void USART0_TX_IRQHandler(void)
{
  // Send a previously received character
  if (*output_ptr != '\0'){
    USART_Tx(USART0, *(output_ptr++));
  }
  else
  {
    free((void*)output_ptr);
    output_ptr = NULL;
    USART_IntDisable(USART0, USART_IEN_TXBL);
  }
  return;
}

/***************************************************************************//**
 * Callback on button change.
 ******************************************************************************/
void sl_button_on_change(const sl_button_t *handle)
{
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
    if (&sl_button_btn0 == handle) {
        printf("btn0 int pressed\r\n");
        //UARTDRV_Transmit(sl_uartdrv_usart_mikroe_handle, (uint8_t*)"@@@@@@@@", 8, NULL);
    }
  }

  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_RELEASED) {
    if (&sl_button_btn0 == handle) {
        printf("btn0 int released\r\n");
        if(output_ptr == NULL){
            output_ptr = malloc(strlen("at\r\n"));
            strcpy((char*)output_ptr, "at\r\n");
            USART_IntEnable(USART0, USART_IEN_TXBL);
            USART0->TXDATA = *output_ptr++;
        }
        else{
            printf("output_ptr not null");
        }
    }
  }
}
