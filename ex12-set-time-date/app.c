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
#include "sl_sleeptimer.h"
/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  printf("\r\nWelcome to the sleeptimer wallclock sample application\r\n");
  printf("Type 'help' to see available commands\r\n");

  sl_sleeptimer_timestamp_t time = sl_sleeptimer_get_time();

  printf("Current time (Unix epoch) is %lu\r\n", time);

  if (SL_STATUS_OK == sl_sleeptimer_set_time(time)) {
    printf("Time set to %lu (Unix epoch)\r\n", time);
  } else {
    printf("Error setting time\r\n");
  }

  sl_sleeptimer_timestamp_t unix_time;

  uint32_t ntp_time;

  // Must convert NTP time to Unix epoch to set time in sleeptimer
  if (SL_STATUS_OK == sl_sleeptimer_convert_ntp_time_to_unix(ntp_time, &unix_time) && (SL_STATUS_OK == sl_sleeptimer_set_time(unix_time))) {
    printf("Time set to %lu (NTP epoch)\r\n", ntp_time);
  } else {
    printf("Error setting time\r\n");
  }

  //Refer to strftime() from UNIX for format rules
  const uint8_t *format = (const uint8_t *)"%Y-%m-%d %T";
  sl_sleeptimer_date_t date;
  char date_string[64];

  if (0 == sl_sleeptimer_get_datetime(&date)
      && 0 != sl_sleeptimer_convert_date_to_str(date_string, sizeof(date_string), format, &date)) {
    printf("Current date is %s\r\n", date_string);
  } else {
    printf("Error getting current date\r\n");
  }


}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

