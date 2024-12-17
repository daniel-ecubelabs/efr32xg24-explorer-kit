/*
 * led.c
 *
 *  Created on: 2024. 12. 9.
 *      Author: Ecubelabs
 */
#include "sl_simple_led.h"
#include "sl_simple_led_instances.h"
#include "sl_sleeptimer.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

#ifndef LED_INSTANCE
#define LED_INSTANCE_0    sl_led_led0
#define LED_INSTANCE_1    sl_led_led1
#endif

#ifndef TOOGLE_DELAY_MS
#define TOOGLE_DELAY_500_MS         500
#define TOOGLE_DELAY_1000_MS        1000
#endif

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/

sl_sleeptimer_timer_handle_t timer_0, timer_1;
volatile bool toggle_timeout_0 = false, toggle_timeout_1 = false;

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/

static void on_timeout_0(sl_sleeptimer_timer_handle_t *handle,
                       void *data);
static void on_timeout_1(sl_sleeptimer_timer_handle_t *handle,
                       void *data);

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * Initialize led blink example.
 ******************************************************************************/
void led_init(void)
{
  // Create timer for waking up the system periodically.
  sl_sleeptimer_start_periodic_timer_ms(&timer_0,
                                        TOOGLE_DELAY_500_MS,
                                        on_timeout_0, NULL,
                                        0,
                                        SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);

  sl_sleeptimer_start_periodic_timer_ms(&timer_1,
                                        TOOGLE_DELAY_1000_MS,
                                        on_timeout_1, NULL,
                                        0,
                                        SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);

}

/***************************************************************************//**
 * Blink ticking function.
 ******************************************************************************/
void led_process_action(void)
{
  if (toggle_timeout_0 == true) {
    sl_led_toggle(&LED_INSTANCE_0);
    toggle_timeout_0 = false;
  }

  if (toggle_timeout_1 == true) {
    sl_led_toggle(&LED_INSTANCE_1);
    toggle_timeout_1 = false;
  }

}


/***************************************************************************//**
 * Sleeptimer timeout callback.
 ******************************************************************************/
static void on_timeout_0(sl_sleeptimer_timer_handle_t *handle,
                       void *data)
{
  (void)&handle;
  (void)&data;
  toggle_timeout_0 = true;
}

static void on_timeout_1(sl_sleeptimer_timer_handle_t *handle,
                       void *data)
{
  (void)&handle;
  (void)&data;
  toggle_timeout_1 = true;
}
