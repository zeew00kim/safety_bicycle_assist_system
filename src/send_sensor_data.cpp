#include "send_sensor_data.hpp"

void sendSensorData(const String &key, const float value) {
  String jsonBuffer = "{\"" + key + "\":" + String(value, 2) + "}\n"; // 측정 값은 소수저 두 번째 자리까지 -> String(value, 2)
  BT_SERIAL.print(jsonBuffer + "\n"); // BLE 모듈로 데이터 전송
  Serial.println(jsonBuffer);         // 시리얼 모니터 확인용 출력
  delay(200); 
}