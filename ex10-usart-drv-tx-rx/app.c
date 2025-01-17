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
#include "sl_uartdrv_instances.h"

static void uart_receive_callback(UARTDRV_Handle_t handle,
                                  Ecode_t transferStatus,
                                  uint8_t *data,
                                  UARTDRV_Count_t transferCount);
void uart_receive(void);

#define UART_RX_BUF_SIZE        16    // 0x100 Read buffer for UART
#define LOCAL_RX_BUF_SIZE       16
#define CIRC_CNT(head, tail, size)   (((head) - (tail)) & ((size) - 1))
#define CIRC_SPACE(head, tail, size) CIRC_CNT((tail), ((head) + 1), (size))

struct data_circ_buf_s
{
  uint16_t    head;
  uint16_t    tail;
  uint8_t        buf[UART_RX_BUF_SIZE];
};

typedef struct data_circ_buf_s data_circ_buf_t;

uint16_t        local_buff_length;                /**< from usb_uart_rx parser to application */
uint8_t         local_buff[LOCAL_RX_BUF_SIZE];      /**< for RX from USB/USART*/
data_circ_buf_t uartRx;
data_circ_buf_t tx_fifo;
char complete = 0;
int uart_transmit(const uint8_t *ptr, int size);
/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  printf("app_iniit\r\n");

  // Enable NVIC USART sources
  NVIC_ClearPendingIRQ(USART0_RX_IRQn);
  NVIC_EnableIRQ(USART0_RX_IRQn);
  NVIC_ClearPendingIRQ(USART0_TX_IRQn);
  NVIC_EnableIRQ(USART0_TX_IRQn);
#define UART_RX_TIMEOUT_CFG                  (USART_TIMECMP1_TSTOP_RXACT    \
                                              | USART_TIMECMP1_TSTART_RXEOF \
                                              | (0xff <<                    \
                                                 _USART_TIMECMP1_TCMPVAL_SHIFT))

  //USART_IntEnable(USART0, USART_IEN_RXDATAV);
  USART0->TIMECMP1 = UART_RX_TIMEOUT_CFG;
  USART_IntEnable(USART0, USART_IF_TCMP1);

  //UARTDRV_Receive(sl_uartdrv_usart_mikroe_handle, &ch, 1, uart_receive_callback);
  uart_receive();
}
typedef enum
{
  cmdREGULAR=0,/* Regular command */
  cmdJSON,/* JSON command */
  cmdUNKNOWN_TYPE/* Unknown command */
}command_type_e;

uint16_t waitForCommand(uint8_t *pBuf,
                          uint16_t len,
                          uint16_t *read_offset,
                          uint16_t cyclic_size)
{
  uint16_t          ret = 0;
  static uint16_t     cmdLen = 0;
  static uint8_t      cmdBuf[32];   /* Commands buffer */
  uint16_t        cnt;
  static command_type_e command_type = cmdUNKNOWN_TYPE;
  static uint8_t      brackets_cnt;

  local_buff_length = 0;

  for (cnt = 0; cnt < len; cnt++)// Loop over the buffer rx data
  {
    if (pBuf[*read_offset] == '\b') { // erase of a char in the terminal
      //port_tx_msg((uint8_t *)"\b\x20\b", 3);
      if (cmdLen) {
        cmdLen--;
      }
    } else {
      //port_tx_msg(&pBuf[*read_offset], 1);
      if ((pBuf[*read_offset] == '\n') || (pBuf[*read_offset] == '\r')) {
        if ((cmdLen != 0) && (command_type == cmdREGULAR)) {// Checks if need to
                                                            //   handle regular
                                                            //   command
          // Need to update the app commands buffer
          memcpy(&local_buff[local_buff_length], cmdBuf, cmdLen);
          local_buff[local_buff_length + cmdLen] = '\n';
          local_buff_length += (cmdLen + 1);
          cmdLen = 0;
          command_type = cmdUNKNOWN_TYPE;
          ret = 1;
          //printf("[1].%s",local_buff);
          uart_transmit(local_buff, local_buff_length);
        }
      } else if (command_type == cmdUNKNOWN_TYPE) {// Need to find out if
                                                   //   getting regular command
                                                   //   or JSON
        cmdBuf[cmdLen] = pBuf[*read_offset];
        if (pBuf[*read_offset] == '{') {// Start Json command
          command_type = cmdJSON;
          brackets_cnt = 1;
        } else {// Start regular command
          command_type = cmdREGULAR;
        }
        cmdLen++;
      } else if (command_type == cmdREGULAR) {// Regular command
        cmdBuf[cmdLen] = pBuf[*read_offset];
        cmdLen++;
      } else {// Json command
        cmdBuf[cmdLen] = pBuf[*read_offset];
        cmdLen++;
        if (pBuf[*read_offset] == '{') {
          brackets_cnt++;
        } else if (pBuf[*read_offset] == '}') {
          brackets_cnt--;
          if (brackets_cnt == 0) {// Got a full Json command
            // Need to update the app commands buffer
            memcpy(&local_buff[local_buff_length], cmdBuf, cmdLen);
            local_buff[local_buff_length + cmdLen] = '\n';
            local_buff_length += (cmdLen + 1);
            cmdLen = 0;
            command_type = cmdUNKNOWN_TYPE;
            ret = 1;
            //printf("[2].%s",local_buff);
            uart_transmit(local_buff, local_buff_length);
          }
        }
      }
    }
    *read_offset = (*read_offset + 1) & cyclic_size;
    if (cmdLen >= sizeof(cmdBuf)) {/* Checks if command too long and we need to
                                    *   reset it */
      cmdLen = 0;
      command_type = cmdUNKNOWN_TYPE;
    }
  }

  if (ret == 1) {// If there is at least 1 command, add 0 at the end
    local_buff[local_buff_length] = 0;
    local_buff_length++;
  }
  return (ret);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  if(complete){
    uint16_t    headUart, tailUart;
  headUart = uartRx.head;
  tailUart = uartRx.tail;

    int uartLen = CIRC_CNT(headUart, tailUart, sizeof(uartRx.buf));
    if(uartLen>0){
//    printf("%d\r\n", uartLen);
//    for(int i=0;i<uartLen;i++){
//      printf("%c", uartRx.buf[tailUart]);
//      tailUart = (tailUart + 1) & (sizeof(uartRx.buf)-1);
//    }
        waitForCommand(uartRx.buf, uartLen, &tailUart, sizeof(uartRx.buf) - 1);
        uartRx.tail = tailUart;

  }
    complete = 0;
  }

}

static uint32_t uart_driver_receive_items_remaining;

static inline void deca_uart_update_fifo(uint32_t received_data_count)
{
  uart_driver_receive_items_remaining -= received_data_count;

  // data is received directly to app.uartRx.buf just update the head
  uartRx.head = (uartRx.head + received_data_count) & (sizeof(uartRx.buf)-1);

//  // Notify upper layer
//  if (app.ctrlTask.Handle) { // RTOS : ctrlTask could be not started yet
//    // signal to the ctrl thread : USB data ready
//    osThreadFlagsSet(app.ctrlTask.Handle, app.ctrlTask.Signal);
//  }
}
void uart_receive(void)
{
    Ecode_t retVal;

  if (/*uart_driver_handle.peripheral.uart              // initialized
      && */(0 == uart_driver_receive_items_remaining)   // receiving is completed
      && (CIRC_SPACE(uartRx.head, uartRx.tail, sizeof(uartRx.buf)) > 0)) {  // have free space
    // Start reception with continuous memory blocks
    int start_idx = uartRx.head;
    int tail = uartRx.tail;
    uart_driver_receive_items_remaining =
      (start_idx
       < tail) ? (tail - start_idx) : (int)sizeof(uartRx.buf) - start_idx;

    //printf("@%d, %d, %ld\r\n",start_idx, tail, uart_driver_receive_items_remaining);
    //printf("3.%d\r\n", sl_uartdrv_usart_mikroe_handle->rxQueue->used);
    retVal = UARTDRV_Receive(sl_uartdrv_usart_mikroe_handle,
                             &uartRx.buf[start_idx],
                    uart_driver_receive_items_remaining,
                    uart_receive_callback);
    if (retVal != ECODE_EMDRV_UARTDRV_OK) {
        printf("uart_receive::UARTDRV_Receive\r\n");
    }
  }

}

static void uart_receive_callback(UARTDRV_Handle_t handle,
                                  Ecode_t transferStatus,
                                  uint8_t *data,
                                  UARTDRV_Count_t transferCount)
{
  (void)handle;
  (void)data;
  (void)transferCount;

  if (ECODE_OK == transferStatus) { // DMA transfer completed
      //printf("1.%d\r\n", sl_uartdrv_usart_mikroe_handle->rxQueue->used);
    deca_uart_update_fifo(uart_driver_receive_items_remaining);
    uart_receive();
  }
}

void USART0_RX_IRQHandler(void)
{

  // Get the character just received
  //ch = USART0->RXDATA;
  //USART0->TXDATA = ch;
  // notify upper layer (receive restart not needed)
  int items_remaining = 0;
  if (ECODE_OK
     == DMADRV_TransferRemainingCount(sl_uartdrv_usart_mikroe_handle->rxDmaCh,
                                      &items_remaining)) {
      deca_uart_update_fifo(uart_driver_receive_items_remaining - items_remaining);
      //printf("%d:%d\r\n", uart_driver_receive_items_remaining, items_remaining);

      //int start_idx = sl_uartdrv_usart_mikroe_handle->rxQueue->head;//app.uartRx.head;
      //int tail = sl_uartdrv_usart_mikroe_handle->rxQueue->tail;//app.uartRx.tail;
      //printf("%d, %d, %ld\r\n", start_idx, tail, uart_driver_receive_items_remaining);
      //printf("2.%d\r\n", sl_uartdrv_usart_mikroe_handle->rxQueue->used);
      complete = 1;
  }

  USART_IntClear(USART0, _USART_IF_MASK);
  //USART0->TXDATA = ch;
}

int uart_transmit(const uint8_t *ptr, int size)
{
  if ((NULL == ptr) || (size <= 0)) {
    return -1;
  }

#if HAL_UART_SEND_BLOCKING
  // Blocking transmit
  for (int i = 0; i < size; i++)
  {
    uart_blocking_tx(HAL_UART_PERIPHERAL, ptr[i]);
  }
  return 0;
#elif HAL_UART_SIMPLE_ASYNCH_SEND
  return UARTDRV_Transmit(&uart_driver_handle, ptr, size, NULL);
#else
  // Will be used as a WR only buffer.
  // ignore overwrite because it should not happen if printing speed and buffer
  //   size is chosen correctly.
  static data_circ_buf_t tx_fifo;

  // We must provide continuous buffer to UARTDRV_Transmit()
  int sc = 0;
  const size_t remaining_space = sizeof(tx_fifo.buf) - tx_fifo.tail;
  if (size > remaining_space) {
    // copy data to static buffer
    memcpy(&tx_fifo.buf[tx_fifo.tail], ptr, remaining_space);
    sc |= UARTDRV_Transmit(sl_uartdrv_usart_mikroe_handle,
                           &tx_fifo.buf[tx_fifo.tail],
                           remaining_space,
                           NULL);
    // Update pointers
    size -= remaining_space;
    ptr += remaining_space;
    tx_fifo.tail = 0;
  }
  if (size) {
    memcpy(&tx_fifo.buf[tx_fifo.tail], ptr, size);
    sc |= UARTDRV_Transmit(sl_uartdrv_usart_mikroe_handle,
                           &tx_fifo.buf[tx_fifo.tail],
                           size,
                           NULL);
    tx_fifo.tail = (tx_fifo.tail + size) & (sizeof(uartRx.buf) - 1);
  }
  return sc;
#endif
}
