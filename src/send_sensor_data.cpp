#include "send_sensor_data.hpp"

void sendSensorData(const String &key, const double value) {
  String jsonBuffer = "{\"" + key + "\":" + String(value, 1) + "}\n";  
  BT_SERIAL.print(jsonBuffer + "\n"); // BLE 모듈로 데이터 전송
  Serial.println(jsonBuffer);         // 시리얼 모니터 확인용 출력
  delay(200); 
}