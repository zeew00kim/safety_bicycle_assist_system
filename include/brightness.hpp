#ifndef BRIGHT_H
#define BRIGHT_H

#include "main.hpp"

// 조도 센서 아날로그 핀
#define CDS_SENSOR_PIN A0

// 전방 및 후방 오토라이트 핀
#define FRONT_LED_PIN 5
#define REAR_LED_PIN 6

#define LIGHT_STATE_CHANGE_TIME 2000 // 오토 라이트 마지막 점등 시점 비교용
#define LIGHT_THRESHOLD         300  // 오토 라이트 점등 임계 조건 (300 이상일 경우 전후방 라이트 점등)

#define FRONT_LIGHT_ON  digitalWrite(FRONT_LED_PIN, HIGH)
#define REAR_LIGHT_ON   digitalWrite(REAR_LED_PIN, HIGH)

#define FRONT_LIGHT_OFF digitalWrite(FRONT_LED_PIN, LOW)
#define REAR_LIGHT_OFF  digitalWrite(REAR_LED_PIN, LOW)


extern uint32_t lightValue;

void cds_data_read_transmit();
void auto_light_control(unsigned long);

#endif