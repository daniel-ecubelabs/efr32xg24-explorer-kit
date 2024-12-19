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
#include "sl_iostream.h"
#include "sl_iostream_init_instances.h"
#include "sl_iostream_handles.h"
//button
#include "sl_simple_button_instances.h"
//timer
#include "sl_sleeptimer.h"

#ifndef BUTTON_INSTANCE_0
#define BUTTON_INSTANCE_0   sl_button_btn0
#endif

#ifndef BUTTON_INSTANCE_1
#define BUTTON_INSTANCE_1   sl_button_btn1
#endif

//#ifndef LED_INSTANCE_0
//#define LED_INSTANCE_0      sl_led_led0
//#endif

/**************************************************************************//**
 * enums, handles, and variables                                              *
 *****************************************************************************/
sl_sleeptimer_timer_handle_t general_timer;
static bool general_timer_running = false;
/**************************************************************************//**
 * Local functions and callbacks
 *****************************************************************************/
static void start_general_timer(void);
static void general_timer_callback(sl_sleeptimer_timer_handle_t *handle,
                            void *data);

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  /* Prevent buffering of output/input.*/
#if !defined(__CROSSWORKS_ARM) && defined(__GNUC__)
  setvbuf(stdout, NULL, _IONBF, 0);   /*Set unbuffered mode for stdout (newlib)*/
  setvbuf(stdin, NULL, _IONBF, 0);   /*Set unbuffered mode for stdin (newlib)*/
#endif

  /* Output on vcom usart instance */
  const char str1[] = "Hello World\r\n";
  sl_iostream_write(sl_iostream_vcom_handle, str1, strlen(str1));

  /* Setting default stream */
  sl_iostream_set_default(sl_iostream_vcom_handle);
  const char str2[] = "This is output on the default stream\r\n";
  sl_iostream_write(SL_IOSTREAM_STDOUT, str2, strlen(str2));

  /* Using printf */
  /* Writing ASCII art to the VCOM iostream */
  printf("Printf uses the default stream, as long as iostream_retarget_stdio is included.\r\n");

  /* Sleep Timer */
  start_general_timer();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  if(general_timer_running == false){
      start_general_timer();
      printf("general timer started \r\n");
  }
}

/***************************************************************************//**
 * Callback on button change.
 ******************************************************************************/
void sl_button_on_change(const sl_button_t *handle)
{
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
    if (&BUTTON_INSTANCE_0 == handle) {
        printf("btn0 pressed\r\n");
    }
    else if (&BUTTON_INSTANCE_1 == handle) {
        printf("btn1 pressed\r\n");
    }
  }

  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_RELEASED) {
    if (&BUTTON_INSTANCE_0 == handle) {
        printf("btn0 released\r\n");
    }
    else if (&BUTTON_INSTANCE_1 == handle) {
        printf("btn1 released\r\n");
    }
  }
}

/**************************************************************************//**
@brief
 *   Starting the global timer, the timeout will restart the BLE activity
 *
 *****************************************************************************/
static void start_general_timer(void){
  sl_sleeptimer_start_timer(&general_timer,
                            sl_sleeptimer_ms_to_tick(3000),
                            general_timer_callback,
                            NULL, 0, 0);
  general_timer_running = true;
}

/***************************************************************************//**
 * @brief
 *   callback of the general timer
 *
 *
 * @param[in] *handle
 *   pointer to the sleeptimer handler
 * ****************************************************************************/
static void general_timer_callback(sl_sleeptimer_timer_handle_t *handle,
                                   void *data)
{
  (void) data;
  (void) handle;
  printf("General timer expired \r\n");
  general_timer_running = false;
}
