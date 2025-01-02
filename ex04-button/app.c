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

#include "sl_simple_button_instances.h"

#ifndef BUTTON_INSTANCE_0
#define BUTTON_INSTANCE_0   sl_button_btn0
#endif

#ifndef BUTTON_INSTANCE_1
#define BUTTON_INSTANCE_1   sl_button_btn1
#endif

//#ifndef LED_INSTANCE_0
//#define LED_INSTANCE_0      sl_led_led0
//#endif
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
  const char str1[] = "IOstream USART example\r\n\r\n";
  sl_iostream_write(sl_iostream_vcom_handle, str1, strlen(str1));

  /* Setting default stream */
  sl_iostream_set_default(sl_iostream_vcom_handle);
  const char str2[] = "This is output on the default stream\r\n";
  sl_iostream_write(SL_IOSTREAM_STDOUT, str2, strlen(str2));

  /* Using printf */
  /* Writing ASCII art to the VCOM iostream */
  printf("Printf uses the default stream, as long as iostream_retarget_stdio is included.\r\n");
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
#if 0 //polling 방식
  switch(sl_simple_button_get_state(SL_SIMPLE_BUTTON_INSTANCE(0)))
  {
    case SL_SIMPLE_BUTTON_PRESSED:
      printf("BTN0 PRESSED\r\n");
      break;

    case SL_SIMPLE_BUTTON_RELEASED:
      printf("BTN0 RELEASE\r\n");
      break;
  }

  if(sl_simple_button_get_state(SL_SIMPLE_BUTTON_INSTANCE(0)) == SL_SIMPLE_BUTTON_PRESSED){
      printf("btn0 pressed\r\n");
  }

  if(sl_simple_button_get_state(SL_SIMPLE_BUTTON_INSTANCE(1)) == SL_SIMPLE_BUTTON_PRESSED){
      printf("btn1 pressed\r\n");
  }
#endif
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
