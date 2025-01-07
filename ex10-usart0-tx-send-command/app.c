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
#include "sl_sleeptimer.h"

#include "circular_queue.h"

volatile uint8_t *output_ptr = NULL;
static Queue_t cmd_q;
sl_sleeptimer_timer_handle_t my_timer;

#define OUT_BUFFER_SIZE 100
#define RX_BUFFER_SIZE 100
#define CMD_MAX_SIZE 100

uint8_t rx_buffer[RX_BUFFER_SIZE] = {0,};
uint8_t rx_buffer_index = 0;
uint8_t line_counter = 0;

//AT 명령어 구조체에 포함되는 명령 콜백함수
//아래 cmd_cb 함수가 호출되면 해당 명령어 구조체에 정의된 콜백 함수가 호출됨
typedef void (*ln_cb_t)(uint8_t *response, uint8_t call_number);

//AT 명령 후 응답을 확인하는 콜백함수
static void cmd_cb(uint8_t *data, uint8_t call_number);

//AT 명령 전송 후 전송에 따른 타임아웃 후에 호출되는 콜백함수
//위에 ln_cb_t 콜백함수와는 다른 함수임
//명령어 타임아웃 전에는 ln_cb_t 가 호출되고, 타임아웃 후에는 timer_cb 가 호출됨
static void timer_cb(sl_sleeptimer_timer_handle_t *handle, void *data);

typedef struct {
  uint8_t cms_string[CMD_MAX_SIZE];
  ln_cb_t ln_cb;
  uint16_t timeout_ms;
} at_cmd_desc_t;

sl_status_t send_cmd();
void send_cmd_cb(uint8_t *new_line, uint8_t call_number);
void usart_register();
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

  //rx 인터럽트 활성화
  USART_IntEnable(USART0, USART_IEN_RXDATAV);

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
  USART_TypeDef *usart0 = USART0;
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
#if 0
  //data = USART_Rx(USART0);
  USART_TypeDef *usart0 = USART0;

  data = usart0->RXDATA;


  //printf("%c", data);
  USART_Tx(USART0, data);
#else
  //if(USART0->IF & USART_STATUS_RXDATAV){
  USART_TypeDef *usart0 = USART0;

  rx_buffer[rx_buffer_index] = usart0->RXDATA;

  if (rx_buffer[rx_buffer_index] == '\r') {
      //ignore \r character
      rx_buffer[rx_buffer_index] = 0;
  }
  else if (rx_buffer[rx_buffer_index] == '\n') {
      rx_buffer[rx_buffer_index] = 0;

      if (rx_buffer_index > 0) {
          cmd_cb(rx_buffer, ++line_counter);
          memset((void*) rx_buffer, 0, RX_BUFFER_SIZE);
          rx_buffer_index = 0;
      }
  }
  else if (rx_buffer[rx_buffer_index] == '>') {
      cmd_cb(rx_buffer, ++line_counter);
      memset(rx_buffer, 0, RX_BUFFER_SIZE);
      rx_buffer_index = 0;
  }
  else if (rx_buffer_index < RX_BUFFER_SIZE - 1) {
      rx_buffer_index++;
  }
  else {
      cmd_cb(rx_buffer, ++line_counter);
      memset(rx_buffer, 0, RX_BUFFER_SIZE);
      rx_buffer_index = 0;
  }

  usart0->IF_CLR = USART_STATUS_RXDATAV;

  //UARTDRV_Receive(sl_uartdrv_usart_mikroe_handle, &data, 1, NULL);
  usart0->CMD = USART_CMD_RXEN;

  ;

#endif
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
        send_cmd();
    }
    if (&sl_button_btn1 == handle) {
        printf("btn1 int released\r\n");
        usart_register();
    }
  }
}

void send_cmd_cb(uint8_t *new_line, uint8_t call_number)
{
  printf("send_cmd_cb++\r\n");

  printf("%s,%d", new_line, call_number);

  printf("send_cmd_cb--\r\n");

  return;
}

sl_status_t send_cmd()
{
  at_cmd_desc_t *at_cmd_descriptor;

  printf("send_cmd++\r\n");

  static at_cmd_desc_t at_ip = { "AT+QIACT?", send_cmd_cb, 3000 };

  printf("push-queue++\r\n");

  if (queueIsFull(&cmd_q)) {
      printf("push_cmd::queue full\r\n");
      return 0;
  }

  queueAdd(&cmd_q, (void*)&at_ip);

  printf("push-queue--\r\n");

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

  line_counter = 0;

  st = sl_sleeptimer_restart_timer_ms(&my_timer, at_cmd_descriptor->timeout_ms, timer_cb, (void*) NULL, 0, 0);

  if (SL_STATUS_OK != st) {
      printf("sl_sleeptimer_restart_timer_ms error\r\n");
  }

  printf("send_cmd--\r\n");

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

  //if (NULL != cmd_cb) {
  //    cmd_cb(NULL, 0);
  //}

  return;
}

/**************************************************************************//**
 * @brief
 *    General platfrom core callback function.
 *    Called in case of new line or timeout.
 *
 * @param[in] data
 *    Pointer to the data received in an new line.
 *
 * @param[in] call_number
 *    Number of received new lines.
 *    Is 0 if timeout occurred
 *
 *****************************************************************************/
static void cmd_cb(uint8_t *data, uint8_t call_number)
{
  at_cmd_desc_t *at_cmd_descriptor = queuePeek(&cmd_q);

  //call number == 0 means timeout occurred
  if (call_number == 0) {
      printf("general_platform_cb::call_number 0\r\n");
      USART_IntDisable(USART0, USART_IEN_TXBL);
      USART_IntDisable(USART0, USART_IEN_RXDATAV);
  }
  else {
    if (at_cmd_descriptor != NULL) {
      // call line callback of the command descriptor if available
      at_cmd_descriptor->ln_cb(data, call_number);
    }
  }
  return;
}

void usart_register()
{
  //USART_TypeDef *usart = USART0;

  for(uint32_t i=0x00; i<=0x068; i+=0x4){
      printf("0x%04lx, 0x%04lx\r\n", i, *((volatile uint32_t*)(USART0_BASE + i)));
  }

  for(uint32_t i=0x1000; i<=0x1068; i+=0x4){
        printf("0x%04lx, 0x%04lx\r\n", i, *((volatile uint32_t*)(USART0_BASE + i)));
    }
  for(uint32_t i=0x2000; i<=0x2068; i+=0x4){
        printf("0x%04lx, 0x%04lx\r\n", i, *((volatile uint32_t*)(USART0_BASE + i)));
    }
  for(uint32_t i=0x3000; i<=0x3068; i+=0x4){
        printf("0x%04lx, 0x%04lx\r\n", i, *((volatile uint32_t*)(USART0_BASE + i)));
    }

}
