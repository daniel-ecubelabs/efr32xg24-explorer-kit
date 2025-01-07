# DMADRV C Example
DMADRV를 이용해서 usart로 수신되는 데이터를 버퍼 크기만큼 입력받아 출력하는 예제
송신과 수신용 버퍼를 전역으로 선언하고, 각 채널별로 DMADRV에 할당하여 사용함.
인터럽트 방식은 콜백함수를 이용
폴링 방식은 DMADRV_TransferActive(tx_channel, &active); active 확인
참고로 IO Stream USART 드라이버를 추가하면 DMADRV도 자동으로 추가됨
(초기화를 없이도 일부는 동작 됨-수신 버퍼 이상으로 데이터 입력)

## Software Components
Platform > Driver > DMADRV
Service > IO Stream > Driver > IO Stream
Service > IO Stream > Driver > IO Stream USART > vcom


