# Button 0(PB02) /Button 1(PB03) C Example
버튼0, 버튼1 예제
Platform > Button > Simple Button 
버튼의 갯수에 따라 Add Instance로 추가 후 Pintool 에서 각 gpio별 버튼을 할당
##Configs
SL_SIMPLE_BUTTON_MODE > Interrupt
버튼 인터럽트가 발생하면 아래 콜백함수가 호출되어 버튼 상태를 확인
sli_simple_button_on_change > sl_button_on_change 