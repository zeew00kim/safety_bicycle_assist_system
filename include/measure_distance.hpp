#ifndef MEASURE_DISTANCE_HPP
#define MEASURE_DISTANCE_HPP

#include "send_sensor_data.hpp"

// 초음파 센서 핀 번호
#define TRIG_PIN 9      // 초음파 방사
#define ECHO_PIN 10     // 물체에 반사된 초음파을 수신

#define TRIG_PIN_OFF   digitalWrite(TRIG_PIN, LOW);
#define TRIG_PIN_ON    digitalWrite(TRIG_PIN, HIGH);

enum class warningState_t : uint8_t {
    WARNING_STATE_OFF = 0,
    WARNING_STATE_ON  = 1
};

void calcAverageDistance();

#endif