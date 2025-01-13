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

static void uart_receive_callback(UARTDRV_Handle_t handle,
                                  Ecode_t transferStatus,
                                  uint8_t *data,
                                  UARTDRV_Count_t transferCount);
void uart_receive(void);
uint8_t ch ;
/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  printf("app_iniit");

  // Enable NVIC USART sources
  NVIC_ClearPendingIRQ(USART0_RX_IRQn);
  NVIC_EnableIRQ(USART0_RX_IRQn);
  NVIC_ClearPendingIRQ(USART0_TX_IRQn);
  NVIC_EnableIRQ(USART0_TX_IRQn);
#define UART_RX_TIMEOUT_CFG                  (USART_TIMECMP1_TSTOP_RXACT    \
                                              | USART_TIMECMP1_TSTART_RXEOF \
                                              | (0xff <<                    \
                                                 _USART_TIMECMP1_TCMPVAL_SHIFT))

  //USART_IntEnable(USART0, USART_IEN_RXDATAV);
  USART0->TIMECMP1 = UART_RX_TIMEOUT_CFG;
  USART_IntEnable(USART0, USART_IF_TCMP1);

  UARTDRV_Receive(sl_uartdrv_usart_mikroe_handle, &ch, 1, uart_receive_callback);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{

}

void uart_receive(void)
{
//  if (uart_driver_handle.peripheral.uart              // initialized
//      && (0 == uart_driver_receive_items_remaining)   // receiving is completed
//      && (CIRC_SPACE(app.uartRx.head, app.uartRx.tail,
//                     sizeof(app.uartRx.buf)) > 0)) {  // have free space
//    // Start reception with continuous memory blocks
//    int start_idx = app.uartRx.head;
//    int tail = app.uartRx.tail;
//    uart_driver_receive_items_remaining =
//      (start_idx
//       < tail) ? (tail - start_idx) : (int)sizeof(app.uartRx.buf) - start_idx;
    UARTDRV_Receive(sl_uartdrv_usart_mikroe_handle,
                    &ch,
                    1,
                    uart_receive_callback);
//  }
    printf("%c \r\n", ch);
}

static void uart_receive_callback(UARTDRV_Handle_t handle,
                                  Ecode_t transferStatus,
                                  uint8_t *data,
                                  UARTDRV_Count_t transferCount)
{
  (void)handle;
  (void)data;
  (void)transferCount;

  if (ECODE_OK == transferStatus) { // DMA transfer completed
    //deca_uart_update_fifo(uart_driver_receive_items_remaining);
    uart_receive();
  }
}

void USART0_RX_IRQHandler(void)
{

  // Get the character just received
  //ch = USART0->RXDATA;
  //USART0->TXDATA = ch;
  // notify upper layer (receive restart not needed)
  int items_remaining = 0;
  if (ECODE_OK
     == DMADRV_TransferRemainingCount(sl_uartdrv_usart_mikroe_handle->rxDmaCh,
                                      &items_remaining)) {
      //deca_uart_update_fifo(uart_driver_receive_items_remaining - items_remaining);
      printf("items_remaining:%d\r\n", items_remaining);
  }

  USART_IntClear(USART0, _USART_IF_MASK);
  //USART0->TXDATA = ch;
}
