#include "main.hpp"

float previousSpeed = 0;
const float BRAKE_THRESHOLD = 2.0; // 속도가 2km 이상 감소할 때
const float ACCEL_THRESHOLD = 1.1;  

const float TILT_ANGLE_THRESHOLD = 45.0;

const unsigned long SENSOR_UPDATE_INTERVAL = 500; // 센서 갱신 주기

const unsigned long MPU_UPDATE_INTERVAL = 500;  
const unsigned long SPEED_UPDATE_INTERVAL = 500;
const unsigned long ACCIDENT_TIME_THRESHOLD = 5000;
const unsigned long BRAKE_LIGHT_DURATION = 4000;
const unsigned long EMERGENCY_BLINK_INTERVAL = 00;

volatile unsigned long lastWheelTime = 0;
volatile float wheelRPM = 0;
const float WHEEL_CIRCUMFERENCE = 2.1; // 휠 둘레 (미터 단위)

unsigned long lastUpdateTime = 0;
unsigned long lastMpuUpdateTime = 0;
unsigned long lastSpeedUpdateTime = 0;
unsigned long accidentStartTime = 0; // 사고 감지 시작 시간
unsigned long brakeLightOnTime = 0; // 브레이크 점등 시작 시간
bool emergencyBlinkState = false;          // 현재 ON/OFF 상태
unsigned long lastEmergencyBlinkTime = 0;  // 마지막 전환 시각

MPU6050 mpu; //

// 상태 변수
bool isTilted = false;
bool isImpactDetected = false;
bool accidentDetected = false; // 사고 감지 여부

void wheelRotationDetected() {
  unsigned long currentTime = millis();
  unsigned long timeDifference = currentTime - lastWheelTime;

  // 자석이 회전하며 인식되는 속도 50ms 이하일 경우 값 무시
  if (timeDifference > 50) {
    wheelRPM = (60.0 / (timeDifference / 1000.0));
    lastWheelTime = currentTime;
  }
}

float calculateSpeed() {
  unsigned long currentTime = millis();

  // 마지막 회전 감지 이후 2초 이후 RPM 0 값으로 초기화
  if (currentTime - lastWheelTime > 2000) {
    wheelRPM = 0;
  }

  return (wheelRPM * WHEEL_CIRCUMFERENCE * 60.0) / 1000.0;
}

void setup() {
  Serial.begin(9600);
  BT_SERIAL.begin(9600);    // BLE 모듈 TX/RX 심볼 변조 속도

  pinMode(TRIG_PIN, OUTPUT);      // 초음파 송출 핀
  pinMode(ECHO_PIN, INPUT);       // 반사파 수신 핀

  pinMode(FRONT_LED_PIN, OUTPUT); // 전조등
  pinMode(REAR_LED_PIN, OUTPUT);  // 후미등

  pinMode(HALL_SENSOR_PIN, INPUT_PULLUP); // RPM 측정 핀

  pinMode(BRAKE_LED_PIN, OUTPUT);     // 브레이크등
  pinMode(EMERGENCY_LED_PIN, OUTPUT); // 비상등

  Wire.begin();       // I2C 통신 버스 초기화
  mpu.initialize();   // MPU6050 센서 자체 초기화

  if (mpu.testConnection()) {
    Serial.println("자이로 센서 연결 성공");
  } else {
    Serial.println("자이로 센서 연결 실패");
  }

  // 휠 스포크의 자석이 홀 센서를 지날 때마다 ISR 호출 (홀 센서 신호를 외부 인터럽트로 감지)
  attachInterrupt(digitalPinToInterrupt(HALL_SENSOR_PIN), wheelRotationDetected, FALLING);
}

void loop() {
  
  unsigned long currentTime = millis();

  if (currentTime - lastUpdateTime >= SENSOR_UPDATE_INTERVAL) {
    
    lastUpdateTime = currentTime;

    // 초음파 센서 거리 측정 및 전송
    calcAverageDistance(); 

    // CDS 센서 데이터 읽기 및 전송
    cds_data_read_transmit();

    // 전후방 Auto Light LED 점등/소등 상태 전송
    auto_light_control(currentTime);
  }

  if (currentTime - lastMpuUpdateTime >= MPU_UPDATE_INTERVAL) {
    lastMpuUpdateTime = currentTime;
    analyzeMPU6050(currentTime);
  }

  if (currentTime - lastSpeedUpdateTime >= SPEED_UPDATE_INTERVAL) {
  lastSpeedUpdateTime = currentTime;

    float currentSpeed = calculateSpeed();
    sendSensorData("SPEED", currentSpeed);

    static bool brakeActive = false;
    static bool initialized = false;

    if (!initialized) {
      previousSpeed = currentSpeed;
      initialized = true;
    }

    float speedDrop = previousSpeed - currentSpeed;

    // 시리얼 출력은 정상 점등/소등 확인 용도
    if (!brakeActive &&
      previousSpeed >= 5.0 &&
      currentSpeed >= 0.0 &&
      speedDrop >= BRAKE_THRESHOLD) {
      digitalWrite(BRAKE_LED_PIN, HIGH);
      brakeLightOnTime = currentTime;
      brakeActive = true;
      Serial.println("BRAKE ON");
    }

    if (brakeActive && (currentTime - brakeLightOnTime >= BRAKE_LIGHT_DURATION)) {
      digitalWrite(BRAKE_LED_PIN, LOW);
      brakeActive = false;
      Serial.println("BRAKE OFF");
    } 

    previousSpeed = currentSpeed;
  }
  if (accidentDetected && isTilted) {
    if (millis() - lastEmergencyBlinkTime >= EMERGENCY_BLINK_INTERVAL) {
      emergencyBlinkState = !emergencyBlinkState;
      digitalWrite(EMERGENCY_LED_PIN, emergencyBlinkState ? HIGH : LOW);
      lastEmergencyBlinkTime = millis();
    }
  } else {
    // 사고 해제되면 비상등 OFF
    digitalWrite(EMERGENCY_LED_PIN, LOW);
    emergencyBlinkState = false;
  }
}

void analyzeMPU6050(unsigned long currentTime) {
  int16_t ax, ay, az, gx, gy, gz;

  if (!mpu.testConnection()) return;
  
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  float accelX = ax / 16384.0, accelY = ay / 16384.0, accelZ = az / 16384.0;
  float totalAccel = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);

  bool impactNow = (totalAccel > ACCEL_THRESHOLD);
  if (impactNow) {
    isImpactDetected = true;  
  }

  sendSensorData("IMPACT", isImpactDetected ? 1 : 0);
  delay(100);

  float tiltAngle = sqrt(pow(atan2(accelY, accelZ) * 180 / PI, 2) + 
                         pow(atan2(accelX, accelZ) * 180 / PI, 2));

  sendSensorData("TILT", tiltAngle);
  delay(100);

  bool tiltNow = (tiltAngle > TILT_ANGLE_THRESHOLD);

  if (tiltNow) {
    isTilted = true;
  }

  if (isTilted) {
    Serial.println("넘어짐 감지");
  }

  if (isTilted && !tiltNow) {
    isTilted = false;
    accidentDetected = false;
    isImpactDetected = false;
    sendSensorData("ACCIDENT", 0);
    Serial.println("정상각도로 복귀 - 사고 감지 해제");
  }

  if (isTilted && isImpactDetected) {
    if (!accidentDetected) {
      accidentStartTime = currentTime;
      accidentDetected = true;
    } else if (currentTime - accidentStartTime >= ACCIDENT_TIME_THRESHOLD) {
      sendSensorData("ACCIDENT", 1);
      Serial.println("사고 발생 감지 (5초 이상 복귀 안됨)");
    }
  }
}

