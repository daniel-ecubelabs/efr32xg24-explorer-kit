/***************************************************************************//**
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *******************************************************************************
 * # Experimental Quality
 * This code has not been formally tested and is provided as-is. It is not
 * suitable for production environments. In addition, this code will not be
 * maintained and there may be no bug maintenance planned for these resources.
 * Silicon Labs may update projects from time to time.
 ******************************************************************************/
#include "sl_common.h"
#include "sl_iostream_handles.h"
#include "sl_bluetooth.h"
#include "gatt_db.h"
#include "app.h"

#include "app_log.h"
#include "app_assert.h"
#include "sl_simple_button_instances.h"
/*******************************************************************************
 *    Local Macros and Definitions
 ******************************************************************************/

/* Set here the operation mode of the SPP device: */
#define SPP_SERVER_MODE    0
#define SPP_CLIENT_MODE    1

#define SPP_OPERATION_MODE SPP_SERVER_MODE

/*Main states */
#define DISCONNECTED       0
#define SCANNING           1
#define FIND_SERVICE       2
#define FIND_CHAR          3
#define ENABLE_NOTIF       4
#define DATA_MODE          5
#define DISCONNECTING      6

#define STATE_ADVERTISING  1
#define STATE_CONNECTED    2
#define STATE_SPP_MODE     3

// SPP service UUID: 4880c12c-fdcb-4077-8920-a450d7f9b907
const uint8_t serviceUUID[16] = { 0x07,
                                  0xb9,
                                  0xf9,
                                  0xd7,
                                  0x50,
                                  0xa4,
                                  0x20,
                                  0x89,
                                  0x77,
                                  0x40,
                                  0xcb,
                                  0xfd,
                                  0x2c,
                                  0xc1,
                                  0x80,
                                  0x48 };

// SPP data UUID: fec26ec4-6d71-4442-9f81-55bc21d658d6
const uint8_t charUUID[16] = { 0xd6,
                               0x58,
                               0xd6,
                               0x21,
                               0xbc,
                               0x55,
                               0x81,
                               0x9f,
                               0x42,
                               0x44,
                               0x71,
                               0x6d,
                               0xc4,
                               0x6e,
                               0xc2,
                               0xfe };

/* maximum number of iterations when polling UART RX data before sending data
 *   over BLE connection
 * set value to 0 to disable optimization -> minimum latency but may decrease
 *   throughput */
#define UART_POLL_TIMEOUT  5000

/*Bookkeeping struct for storing amount of received/sent data  */
typedef struct
{
  uint32_t num_pack_sent;
  uint32_t num_bytes_sent;
  uint32_t num_pack_received;
  uint32_t num_bytes_received;
  int32_t num_writes; /* Total number of send attempts */
  uint32_t trans_enable;
  uint16_t max_mtu_out;
} ts_counters;

/* Common local functions*/
static void print_stats(ts_counters *p_counters);
static void reset_variables();
static void send_spp_data();

/******************************************************************************
 *    Local Variables
 ******************************************************************************/

// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;
static uint8_t conn_handle = 0xFF;
static uint8_t main_state;
static uint32_t service_handle;
static uint16_t char_handle;

ts_counters counters;

// Default maximum packet size is 20 bytes. This is adjusted after connection is
// opened based on the connection parameters
static uint8_t max_packet_size = 20;
static uint8_t min_packet_size = 20;  // Target minimum bytes for one packet

static void reset_variables()
{
  conn_handle = 0xFF;
  main_state = STATE_ADVERTISING;
  service_handle = 0;
  char_handle = 0;
  max_packet_size = 20;

  memset(&counters, 0, sizeof(counters));
}
//static uint8_t transfer = 0;
static uint8_t *transdata = NULL;
//static int32_t total_length = 50*1024;
//static uint32_t write_offset = 0;

/**************************************************************************
 *    Application Init code
 ***************************************************************************/
void app_init(void)
{
  uint16_t i = 0 ;

  app_log("app_init++\r\n");

  reset_variables();

  counters.num_writes = 50*1024;

  transdata = malloc(counters.num_writes);

  if(transdata != NULL){
      app_log("memory allcated\r\n");
      for(i=0;i<counters.num_writes;i++){
          transdata[i] = i%0xFF;
      }
  }
  app_log("app_init--\r\n");
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/

void app_process_action(void)
{
  size_t len = 0;
  sl_status_t result;

  if (STATE_SPP_MODE == main_state) {
    if(counters.trans_enable == 0 && sl_simple_button_get_state(SL_SIMPLE_BUTTON_INSTANCE(0)) == SL_SIMPLE_BUTTON_PRESSED){
        app_log("btn0 pressed\r\n");
        counters.trans_enable = 1;
    }
    if(counters.trans_enable == 1 && sl_simple_button_get_state(SL_SIMPLE_BUTTON_INSTANCE(1)) == SL_SIMPLE_BUTTON_PRESSED){
        app_log("btn1 pressed\r\n");
        counters.trans_enable = 0;
    }

    if(counters.trans_enable){
        if(counters.num_writes>0){
            if(counters.num_writes<counters.max_mtu_out){
                len = counters.num_writes;
            }
            else{
                len = counters.max_mtu_out;
            }
            counters.num_writes -= counters.max_mtu_out;

            app_log("max=%d\r\n", counters.max_mtu_out);
            app_log("len=%d\r\n", len);
            app_log("write_offset=%ld\r\n", counters.num_bytes_sent);
            // Stack may return "out-of-memory" (SL_STATUS_NO_MORE_RESOURCE) error if
            //   the local buffer is full -> in that case, just keep trying until the
            //   command succeeds
            do {
                result = sl_bt_gatt_server_send_notification(conn_handle,
                                                         gattdb_spp_data,
                                                         len,
                                                         &transdata[counters.num_bytes_sent]);
                //counters.num_writes++;
            } while (result == SL_STATUS_NO_MORE_RESOURCE);

            if (result != 0) {
                app_log("Unexpected error: %lu\r\n", result);
            } else {
              counters.num_bytes_sent += len;
            }
            sl_udelay_wait(1000);
        }
      }
      //send_spp_data();
    }
}

/**************************************************************************//**
 * Bluetooth stack event handler for SPP Server mode
 *
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;

  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:
      app_log("SPP Role: SPP Server\r\n");
      //reset_variables();
      sc = sl_bt_gatt_server_set_max_mtu(247, &counters.max_mtu_out);
      app_assert_status(sc);
      // Create an advertising set.
      sc = sl_bt_advertiser_create_set(&advertising_set_handle);
      app_assert_status(sc);
      // Generate data for advertising
      sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,
                                                 sl_bt_advertiser_general_discoverable);
      app_assert_status(sc);

      // 3. Set PHY (primary 1M, secondary 2M)
//      sl_bt_advertiser_set_phy(
//          advertising_set_handle,
//          sl_bt_gap_1m_phy,      // primary
//          sl_bt_gap_2m_phy);     // secondary (actual data on 2M PHY)


      // Set advertising interval to 100ms.
      sc = sl_bt_advertiser_set_timing(
        advertising_set_handle,
        160,   // min. adv. interval (milliseconds * 1.6)
        160,   // max. adv. interval (milliseconds * 1.6)
        0,     // adv. duration
        0);    // max. num. adv. events
      app_assert_status(sc);
      // Start  advertising and enable connections
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_legacy_advertiser_connectable);
      app_assert_status(sc);

      break;

    // -------------------------------
    // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
      conn_handle = evt->data.evt_connection_opened.connection;
      app_log("Connection opened\r\n");
      main_state = STATE_CONNECTED;

      // Request connection parameter update.
      // conn.interval min 20ms, max 40ms, slave latency 4 intervals,
      // supervision timeout 2 seconds
      // (These should be compliant with Apple Bluetooth Accessory Design
      // Guidelines, both R7 and R8)
      sl_bt_connection_set_parameters(conn_handle, 24, 40, 0, 200, 0, 0xFFFF);
      break;

    case sl_bt_evt_connection_parameters_id:
      app_log("Conn.parameters: interval %u units\r\n",
              evt->data.evt_connection_parameters.interval);
      break;

    case sl_bt_evt_gatt_mtu_exchanged_id:
      // Calculate maximum data per one notification / write-without-response,
      // this depends on the MTU. up to ATT_MTU-3 bytes can be sent at once.
      max_packet_size = evt->data.evt_gatt_mtu_exchanged.mtu - 3;

      /* Try to send maximum length packets whenever possible */
      min_packet_size = max_packet_size;
      counters.max_mtu_out = max_packet_size;
      app_log("MTU exchanged: %d\r\n", evt->data.evt_gatt_mtu_exchanged.mtu);
      break;

    case sl_bt_evt_connection_closed_id:
      print_stats(&counters);
      if (STATE_SPP_MODE == main_state) {
        sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
      }
      reset_variables();
      // Restart advertising after client has disconnected.
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_legacy_advertiser_connectable);
      break;

    case sl_bt_evt_gatt_server_characteristic_status_id:
    {
      sl_bt_evt_gatt_server_characteristic_status_t char_status;
      char_status = evt->data.evt_gatt_server_characteristic_status;

      if (char_status.characteristic == gattdb_spp_data) {
        if (char_status.status_flags == sl_bt_gatt_server_client_config) {
          // Characteristic client configuration (CCC) for spp_data has been
          //   changed
          if (char_status.client_config_flags
              == sl_bt_gatt_server_notification) {
            main_state = STATE_SPP_MODE;
            sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
            app_log("SPP Mode ON\r\n");
          } else {
            app_log("SPP Mode OFF\r\n");
            main_state = STATE_CONNECTED;
            sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
          }
        }
      }
    }
    break;

    case sl_bt_evt_gatt_server_attribute_value_id:
    {
      if (evt->data.evt_gatt_server_attribute_value.value.len != 0) {
        for (uint8_t i = 0;
             i < evt->data.evt_gatt_server_attribute_value.value.len; i++) {
          sl_iostream_putchar(
            sl_iostream_vcom_handle,
            evt->data.evt_gatt_server_attribute_value.value.data[i]);
        }
        counters.num_pack_received++;
        counters.num_bytes_received +=
          evt->data.evt_gatt_server_attribute_value.value.len;
      }
    }
    break;

    // -------------------------------
    // Default event handler.
    default:
      break;
  }
}

static void print_stats(ts_counters *p_counters)
{
  app_log("Outgoing data:\r\n");
  app_log(" bytes/packets sent: %lu / %lu ",
          p_counters->num_bytes_sent,
          p_counters->num_pack_sent);
  app_log(", num writes: %lu\r\n", p_counters->num_writes);
  app_log("(RX buffer overflow is not tracked)\r\n");
  app_log("Incoming data:\r\n");
  app_log(" bytes/packets received: %lu / %lu\r\n",
          p_counters->num_bytes_received,
          p_counters->num_pack_received);

  return;
}

static void send_spp_data()
{
  uint8_t len = 0;
  uint8_t data[256];
  sl_status_t result, read_result;
  uint32_t timeout = 0;
  char c;

  // Read up to max_packet_size characters from local buffer
  while (len < max_packet_size) {
    read_result = sl_iostream_getchar(sl_iostream_vcom_handle, &c);
    if (SL_STATUS_OK == read_result) {
      data[len++] = (uint8_t)c;
    } else if (len == 0) {
      /* If the first ReadChar() fails then return immediately */
      return;
    } else {
      // Speed optimization: if there are some bytes to be sent but the length
      // is still below the preferred minimum packet size, then wait for
      // additional bytes until timeout. Target is to put as many bytes as
      // possible into each air packet.

      // Conditions for exiting the while loop and proceed to send data:
      if (timeout++ > UART_POLL_TIMEOUT) {
        break;
      } else if (len >= min_packet_size) {
        break;
      }
    }
  }

  if (len > 0) {
    // Stack may return "out-of-memory" (SL_STATUS_NO_MORE_RESOURCE) error if
    //   the local buffer is full -> in that case, just keep trying until the
    //   command succeeds
    do {
      result = sl_bt_gatt_server_send_notification(conn_handle,
                                                   gattdb_spp_data,
                                                   len,
                                                   data);
      //counters.num_writes++;
    } while (result == SL_STATUS_NO_MORE_RESOURCE);

    if (result != 0) {
      app_log("Unexpected error: %lu\r\n", result);
    } else {
      //counters.num_pack_sent++;
      //counters.num_bytes_sent += len;
    }
  }
  return;
}
