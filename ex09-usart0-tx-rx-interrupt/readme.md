# UART TX/RX Interrupt C Example
uart0를 통해 입/출력 데이터 인터럽트로 송수신

## Software Components
Application > Utility > Log : 디버그 메시지 출력을 위한 uart 포트 확보
Third party > Segger > Tiny printf : printf를 통한 메시지 출력을 위한 컴포넌트
Platform > Driver > UART > UARTDRV USART : 외부 데이터 입출력을 위한 Uart 확보
Platform > Driver > Button > Simple Button : BTN0을 누르면 외부 uart 출력 기능을 위한 컴포넌트

### 동작 설명

1. USART RX 인터럽트 활성화
	- usart irq 활성화
   	// Enable NVIC USART sources
	NVIC_ClearPendingIRQ(USART0_RX_IRQn);
  	NVIC_EnableIRQ(USART0_RX_IRQn);
//
//  NVIC_ClearPendingIRQ(USART0_TX_IRQn);
//  NVIC_EnableIRQ(USART0_TX_IRQn);
//
	- usart rx 인터럽트 활성화
  	USART_IntEnable(USART0, USART_IEN_RXDATAV);
//  USART_IntEnable(USART0, USART_IEN_TXBL);

	- 인터럽트 활성화를 우해 1회 호출해 주어야 함.
  	UARTDRV_Receive(sl_uartdrv_usart_mikroe_handle, &ch, 1, NULL);
  	
** Usartdrv는 소프트웨어적인 uart 드라이버를 구현한 파일이며, USART_IntEnable 함수들은 하드웨어를 직접 제어하는 코드
	void USART0_RX_IRQHandler(void) 함수안에서 UARTDRV_Receive() 함수 호출을 통해 값을 읽는 방식은 
	소프트웨어와 하드웨어 드라이버 구조가 혼합된 구조로 바람직하지 않음. 

 2. USART TX 인터럽트 활성화
 	- 버튼0이 눌렸다 떼면 메모리 할당 후 Tx 레지스터에 할당된 메모리에 있는 데이터 1바이트가 전송
 	- tx 인터럽트가 발생할 시점에 USART_IntEnable(USART0, USART_IEN_TXBL); 함수를 호출함
 	- tx 인터럽트 핸들러에서 데이터가 모두 전송되면 할당된 메모리 해제 
 