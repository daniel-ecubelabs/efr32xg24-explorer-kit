# UART0 TX Send Command C Example
- uart0를 통해 명령어를 전송

## Software Components
- Application > Utility > Log : 디버그 메시지 출력을 위한 euart0 포트로 자동 지정됨
- Third party > Segger > Tiny printf : printf를 통한 메시지 출력을 위한 컴포넌트
- Platform > Driver > UART > UARTDRV USART : 외부 데이터 입출력을 위한 Uart 확보
- Platform > Driver > Button > Simple Button : BTN0/1 명령어를 원형큐에 넣기 위한 컴포넌트
- Platform > Utilities > Circular queue : 송신하는 명령어가 저장되는 메모리

### 동작 설명
1. 버튼0을 눌렸다 떼면 전송하는 명령어를 원형큐에 저장
2. 버튼1을 눌렀다 떼면 원형큐에 있는 명령어를 뽑아서 uart0으로 전송
   (1바이트를 uart 레지스터에 저장 후 tx 인터럽트 활성화로 나머지는 tx isr에서 전송)


 