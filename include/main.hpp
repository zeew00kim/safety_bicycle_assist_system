#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>
#include "send_sensor_data.hpp"
#include "measure_distance.hpp"

#define BT_SERIAL Serial1  
#define CDS_SENSOR_PIN A0
#define LED_PIN 6
#define WARNING_DISTANCE 50
#define HALL_SENSOR_PIN 2
#define BRAKE_LED_PIN 7
#define EMERGENCY_LED_PIN 4
#define ADDITIONAL_LED_PIN 5

extern float previousSpeed;
extern const float BRAKE_THRESHOLD;
extern const float ACCEL_THRESHOLD;  

extern const float TILT_ANGLE_THRESHOLD;

extern const int LIGHT_THRESHOLD;        

extern const unsigned long SENSOR_UPDATE_INTERVAL;

extern const unsigned long MPU_UPDATE_INTERVAL;
extern const unsigned long SPEED_UPDATE_INTERVAL;
extern const unsigned long ACCIDENT_TIME_THRESHOLD;
extern const unsigned long BRAKE_LIGHT_DURATION;
extern const unsigned long EMERGENCY_BLINK_INTERVAL;

extern volatile unsigned long lastWheelTime;
extern volatile float wheelRPM;
extern const float WHEEL_CIRCUMFERENCE; // 휠 둘레 (미터 단위)

extern unsigned long lastUpdateTime;
extern unsigned long lastMpuUpdateTime;
extern unsigned long lastSpeedUpdateTime;
extern unsigned long accidentStartTime; // 사고 감지 시작 시간
extern unsigned long brakeLightOnTime; // 브레이크 점등 시작 시간
extern bool emergencyBl;          // 현재 ON/OFF 상태
extern unsigned long lastEmergencyBlinkTime;  // 마지막 전환 시각

extern MPU6050 mpu; //

// 상태 변수
extern bool isTilted;
extern bool isImpactDetected;
extern bool accidentDetected; // 사고 감지 여부

// function list
void analyzeMPU6050(unsigned long currentTime);