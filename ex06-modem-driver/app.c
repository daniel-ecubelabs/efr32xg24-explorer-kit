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

  printf("app_init--\r\n");
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
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
