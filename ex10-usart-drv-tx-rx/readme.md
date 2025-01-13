# USART DRV를 이용한 TX/RX C Example
SI에서 제공하는 소프트웨어 컴포넌트 중 USARTDRV를 이용한 TX/RX 예제
## Software Components
Platform > Driver > EUSART : vcom
Platform > Driver > USART : mikroe 
Services > IO Stream > Driver > IO Stream : printf 츨력을 위한 컴포넌트
Services > IO Stream > Driver > IO Stream: Retarget STDIO : printf 츨력을 위한 컴포넌트
Services > IO Stream > Driver > IO Stream: STDLIB Configuration : printf 츨력을 위한 컴포넌트

### 동작 설명
1. USART DRV를 이용
2. 데이터 수신은 UARTDRV_Receive 함수에 콜백 함수가 등록됨.
3. 인터럽트는 UART 타이머 타임아웃, DMA 완료일 때 발생
