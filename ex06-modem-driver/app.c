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
#include <string.h>
#include "sl_uartdrv_instances.h"

#include "at_parser_core.h"

// timer handles, variables and flags
sl_sleeptimer_timer_handle_t bg96_state_timeout_timer;
//static sl_sleeptimer_timer_handle_t bg96_timer;

static sl_status_t bg96_status = SL_STATUS_FAIL;

typedef enum{
  wake_up,
  net_read_imei,
  network_registration,
  GPS_get_position,
  net_open,
  net_send,
  net_close,
  net_delay_open,
  net_delay_netreg,
  net_delay_gsm_start,
  net_delay,
  all_finished
}bg96_state;

bg96_state _bg96_state = wake_up;

at_scheduler_status_t output_object = { SL_STATUS_OK, 0, "" };
/**************************************************************************//**
 * Local functions and callbacks
 *****************************************************************************/
static void start_bg_96_timer(at_scheduler_status_t *output, uint32_t ms);
static void bg96_state_machine_timer_callback(sl_sleeptimer_timer_handle_t *handle,void *data);

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

  at_parser_init();

  start_bg_96_timer(&output_object, 1000);

  at_parser_init_output_object(&output_object);

  printf("app_init--\r\n");
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  at_parser_process();

  if (output_object.status == SL_STATUS_OK) {
    switch (_bg96_state) {
    case wake_up:
      printf("bg96 wake_up finished \r\n");
      _bg96_state = net_read_imei;
      at_parser_init_output_object(&output_object);
      start_bg_96_timer(&output_object, 500);
      break;

    case net_read_imei:
      printf("modem read imei\r\n");
      _bg96_state = net_delay_netreg;
      at_parser_init_output_object(&output_object);
      bg96_status = read_imei(&output_object);
      break;

    case net_delay_netreg:
      printf("bg96 network registration started \r\n");
      _bg96_state = network_registration;
      at_parser_init_output_object(&output_object);
      bg96_status = bg96_network_registration(&output_object);
      break;
    }
  }
}

/**************************************************************************//**
 * @brief
 *   Start a one-shot timer in bg96 state, to wait some for the proper answer
 * @param[in] ms
 *   timer in millisecond
 * @param[in] *output
 *   pointer to the output object of the scheduler
 *****************************************************************************/
static void start_bg_96_timer(at_scheduler_status_t *output, uint32_t ms)
{
  sl_sleeptimer_start_timer(&bg96_state_timeout_timer,
      sl_sleeptimer_ms_to_tick(ms), bg96_state_machine_timer_callback, output,
      0, 0);

//  sl_sleeptimer_stop_timer(&bg96_timer);
//  sl_sleeptimer_restart_timer_ms(&bg96_timer, BG96_TIMEOUT_MS, bg96_timer_cb,(void*) NULL, 0, 0);
}

/***************************************************************************//**
 * @brief
 *   callback of the one-shot timer in bg96 state
 *   will set a flag at the end of the period
 * @param[in] *handle
 *   pointer to the sleeptimer handler
 *****************************************************************************/
static void bg96_state_machine_timer_callback(sl_sleeptimer_timer_handle_t *handle,
                                       void *data){
  (void) handle;
  at_scheduler_status_t *t_output = (at_scheduler_status_t*) data;
  printf("bg96 state machine timer expired \r\n");
  t_output->status = SL_STATUS_OK;
}
