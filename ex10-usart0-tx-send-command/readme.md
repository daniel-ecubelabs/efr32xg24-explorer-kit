# UART0 TX Send Command C Example
- uart0를 통해 명령어를 전송
- 인터럽트 처리부분에서 단순히 입력을 수신하고 출력하는 부분은 계속해서 인터럽트가 처리되는 반면
  원형큐 부분을 처리하고자 하면 인터럽트가 한 번 들어오고 처리되지 않음.
## Software Components
- Application > Utility > Log : 디버그 메시지 출력을 위한 euart0 포트로 자동 지정됨
- Third party > Segger > Tiny printf : printf를 통한 메시지 출력을 위한 컴포넌트
- Platform > Driver > UART > UARTDRV USART : 외부 데이터 입출력을 위한 Uart 확보
- Platform > Driver > Button > Simple Button : BTN0/1 명령어를 원형큐에 넣기 위한 컴포넌트
- Platform > Utilities > Circular queue : 송신하는 명령어가 저장되는 메모리

### 동작 설명
1. 버튼0을 눌렸다 떼면 전송하는 명령어를 원형큐에 삽입

2. 버튼1을 눌렀다 떼면 원형큐에 있는 명령어를 찾아서 uart0으로 전송
	(1바이트를 uart 레지스터에 저장 후 tx 인터럽트 활성화로 나머지는 tx isr에서 전송)

3. 명령이 전송되고, 명령 타임아웃 후 명령 콜백함수 호출
	(명령 전송 후 타임아웃 동안 응답을 기다리고 없으면 명령 콜백이 아닌 전송 콜백에 의한 원형큐 명령어 제거)
	(명령 콜백은 명령어 구조체에 포함된 콜백함수)

 