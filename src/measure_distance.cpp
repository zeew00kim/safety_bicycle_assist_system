#include "measure_distance.hpp"

double measureDistance() {

  // 측정 가능 거리를 벗어난 경우를 대비한 마지막 측정 거리
  static double lastValidDistance = 100;

  TRIG_PIN_OFF;
  delayMicroseconds(2);

  TRIG_PIN_ON;
  delayMicroseconds(10);

  TRIG_PIN_OFF;

  // 왕복 거리 시간 계산 (단위: 마이크로초), Timeout 50ms 이상일 경우 측정 실패로 간주
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 50000);

  if (duration == 0) return lastValidDistance;

  double distance = (duration * 0.0343) / 2.0;
  
  // 측정 거리가 4m 초과 또는 0 미만일 경우 1m 반환
  if (distance > 400 || distance < 0) return lastValidDistance;

  lastValidDistance = distance; 

  return distance;
}

void calcAverageDistance() {
  
  double totalDistance = 0;
  double distance = 0;

  uint8_t validSamples = 0;
  constexpr uint8_t sampleCount = 5;

  warningState_t warningState = warningState_t::WARNING_STATE_OFF;

  for (uint8_t i = 0; i < sampleCount; i++) {
    distance = measureDistance();
    if (distance != -1) {
      totalDistance += distance;
      validSamples++;
    }
    delay(10);
  }

  // 예외 값 대비 5회 측정 값의 평균을 계산
  distance = (validSamples == 0) ? -1 : totalDistance / validSamples;

  // JSON 포맷 변환 후 BLE 모듈로 전송
  sendSensorData("DISTANCE", distance);  
  delay(100);  
    
  if (distance > 0 && distance <= WARNING_DISTANCE) warningState = warningState_t::WARNING_STATE_ON;

  // JSON 포맷 변환 후 BLE 모듈로 전송
  sendSensorData("WARNING", static_cast<int>(warningState));
  delay(100);
}  