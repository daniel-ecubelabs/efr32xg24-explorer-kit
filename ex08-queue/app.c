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
#include "sl_simple_button_instances.h"
#include "circular_queue.h"

//#define CMD_Q_SIZE 20
#define OUT_BUFFER_SIZE 100
#define IN_BUFFER_SIZE 100
#define CMD_MAX_SIZE 100

typedef void (*ln_cb_t)(uint8_t *response, uint8_t call_number);

typedef struct {
  uint8_t cms_string[CMD_MAX_SIZE];
  ln_cb_t ln_cb;
  uint16_t timeout_ms;
} at_cmd_desc_t;

static Queue_t cmd_q;
static uint32_t g_count = 0;

void call_back_func(uint8_t *new_line, uint8_t call_number)
{
  printf("new_line=%s, call_number=%d\r\n", new_line, call_number);

  return;
}
/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  printf("app_init\r\n");

  queueInit(&cmd_q, CIRCULAR_QUEUE_LEN_MAX);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  //at_cmd_desc_t *at_cmd_descriptor;

  if (queueIsEmpty(&cmd_q)) {
      printf("queue empty\r\n");
      return;
  }

  if (queueIsFull(&cmd_q)) {
      printf("queue full\r\n");
      return;
  }
}
/***************************************************************************//**
 * Callback on button change.
 ******************************************************************************/
void sl_button_on_change(const sl_button_t *handle)
{
  at_cmd_desc_t *at_cmd_descriptor;

  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
    if (&sl_button_btn0 == handle) {
        printf("btn0 int pressed\r\n");

        static at_cmd_desc_t at_qgps = { "", call_back_func,  1000 };

        sprintf((char*) at_qgps.cms_string, "count:%d", (int) g_count++);

        //만약 버튼1이 입력되면 큐에 명령어 삽입
        queueAdd(&cmd_q, (void*)&at_qgps);
    }
    else if (&sl_button_btn1 == handle) {
        printf("btn1 int pressed\r\n");

        //만약 버튼 2이 입력되면 큐에 명령어 토출
        at_cmd_descriptor = queuePeek(&cmd_q);

        printf("cmd:%s, time:%d\r\n", at_cmd_descriptor->cms_string, at_cmd_descriptor->timeout_ms);

        queueRemove(&cmd_q);
    }
  }

  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_RELEASED) {
    if (&sl_button_btn0 == handle) {
        printf("btn0 int released\r\n");
    }
    else if (&sl_button_btn1 == handle) {
        printf("btn1 int released\r\n");
    }
  }
}
