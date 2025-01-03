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
#include "circular_queue.h"
#include "sl_sleeptimer.h"

volatile uint8_t *output_ptr = NULL;
static Queue_t cmd_q;
sl_sleeptimer_timer_handle_t my_timer;

#define OUT_BUFFER_SIZE 100
#define IN_BUFFER_SIZE 100
#define CMD_MAX_SIZE 100

#define validate(error, fn_result) error=(SL_STATUS_OK == error)?(fn_result):(error)

typedef void (*ln_cb_t)(uint8_t *response, uint8_t call_number);
static void timer_cb(sl_sleeptimer_timer_handle_t *handle, void *data);

typedef struct {
  uint8_t cms_string[CMD_MAX_SIZE];
  ln_cb_t ln_cb;
  uint16_t timeout_ms;
} at_cmd_desc_t;

void push_cmd();
sl_status_t send_cmd();
void send_cmd_cb(uint8_t *new_line, uint8_t call_number);
/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  uint8_t ch = 0;

  printf("app_init\r\n");

//  static char str[16] = "Hello World\r\n";
//  UARTDRV_Transmit(sl_uartdrv_usart_mikroe_handle, (uint8_t*)str, 16, NULL);

  // Enable NVIC USART sources
  NVIC_ClearPendingIRQ(USART0_RX_IRQn);
  NVIC_EnableIRQ(USART0_RX_IRQn);
  //
  NVIC_ClearPendingIRQ(USART0_TX_IRQn);
  NVIC_EnableIRQ(USART0_TX_IRQn);

  //인터럽트 활성화를 위해 1회 호출해 주어야 함.
  UARTDRV_Receive(sl_uartdrv_usart_mikroe_handle, &ch, 1, NULL);

  //원형큐 초기화
  queueInit(&cmd_q, CIRCULAR_QUEUE_LEN_MAX);
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
  else{
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
        push_cmd();
    }
    else if (&sl_button_btn1 == handle) {
        printf("btn1 int released\r\n");
        send_cmd();
    }
  }
}

void send_cmd_cb(uint8_t *new_line, uint8_t call_number)
{
  printf("send_cmd_cb\r\n");

  printf("%s,%d", new_line, call_number);

  return;
}

void push_cmd()
{
  static at_cmd_desc_t at_ip = { "AT+QIACT?", send_cmd_cb, 1000 };

  printf("push_cmd\r\n");

  if (queueIsFull(&cmd_q)) {
      printf("push_cmd::queue full\r\n");
      return;
  }

  queueAdd(&cmd_q, (void*)&at_ip);

  return;
}

sl_status_t send_cmd()
{
  at_cmd_desc_t *at_cmd_descriptor;

  printf("send_cmd\r\n");

  if (queueIsEmpty(&cmd_q)) {
      printf("queue empty");
      return 0;
  }

  at_cmd_descriptor = queuePeek(&cmd_q);

  sl_status_t st;
  size_t cmd_length = strlen((const char*) at_cmd_descriptor->cms_string);

  if (strstr((const char*)at_cmd_descriptor->cms_string, (const char*)"\r\n") == NULL) {
    if (cmd_length < OUT_BUFFER_SIZE - 2) {
      strcat((char*) at_cmd_descriptor->cms_string, "\r\n");
    }
    else {
        printf("cmd_length over");
        return 0;
    }
  }

  output_ptr = at_cmd_descriptor->cms_string;

  USART_IntEnable(USART0, USART_IEN_TXBL);
  USART_IntEnable(USART0, USART_IEN_RXDATAV);

  USART0->TXDATA = *output_ptr++;

  st = sl_sleeptimer_start_timer_ms(&my_timer, at_cmd_descriptor->timeout_ms, timer_cb, (void*) NULL, 0, 0);

  return st;
}

/**************************************************************************//**
 * @brief
 *   Timeout handler function.
 *
 * @param[in] handle
 *    Timer handler which called the callback function.
 *
 * @param [in] data
 *    Timer data sent by the caller of callback function.
 *
 *****************************************************************************/
static void timer_cb(sl_sleeptimer_timer_handle_t *handle,
                     void *data)
{
  (void) (data);
  (void) handle;

  printf("timer call back\r\n");

  if (queueIsEmpty(&cmd_q)) {
      printf("timer_cb::queue empty\r\n");
      return;
  }

  queueRemove(&cmd_q);
  sl_sleeptimer_stop_timer(&my_timer);

  return;
}
