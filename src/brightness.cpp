#include "brightness.hpp"        

void cds_data_read_transmit() {

    // CDS 조도 센서 측정 데이터 읽기
    uint32_t lightValue = analogRead(CDS_SENSOR_PIN);

    // 읽어온 값 JSON 형태 변환 후 BLE 모듈로 전송
    sendSensorData("LIGHT", lightValue); 
    delay(100);
}

void auto_light_control(unsigned long currentTime) {
    
    // static (전원 공급 즉시 Flash -> RAM DATA Section, 함수 호출마다 초기화 X)
    static unsigned long lastLightChange = 0; 

    // 전후방 라이트 점등 이후 2초가 경과한 경우에만 ON/OFF 토글 진행
    if (currentTime - lastLightChange > LIGHT_STATE_CHANGE_TIME) {
        if (lightValue > LIGHT_THRESHOLD) {
            FRONT_LIGHT_ON;
            REAR_LIGHT_ON;
            sendSensorData("LED", 1);
        } 
        else {
            FRONT_LIGHT_OFF;
            REAR_LIGHT_OFF;
            sendSensorData("LED", 0);
        }
        lastLightChange = currentTime;
    }
    delay(100);
}