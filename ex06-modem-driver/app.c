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

  printf("Printf uses the default stream, as long as iostream_retarget_stdio is included.\r\n");
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}
