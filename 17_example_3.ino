#include <Servo.h>

// Arduino pin assignment
#define PIN_IR    A0        // IR sensor at Pin A0
#define PIN_LED   11
#define PIN_SERVO 10

// Servo pulse width (microseconds)
#define _DUTY_MIN  1000     // servo full clock-wise position (0 degree)
#define _DUTY_NEU  1500     // servo neutral position (90 degree)
#define _DUTY_MAX  2000     // servo full counter-clockwise position (180 degree)

// Distance range (mm): 10~25cm -> 100~250 mm
#define _DIST_MIN  100.0    // minimum distance 100 mm (10 cm)
#define _DIST_MAX  250.0    // maximum distance 250 mm (25 cm)

// EMA alpha and loop interval
#define EMA_ALPHA  0.20     
#define LOOP_INTERVAL 20    // Loop Interval (unit: msec) - 20ms

Servo myservo;
unsigned long last_loop_time;  

float dist_prev = _DIST_MIN;
float dist_ema  = _DIST_MIN;

void setup()
{
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  myservo.attach(PIN_SERVO);
  myservo.writeMicroseconds(_DUTY_NEU);

  Serial.begin(1000000);    // 1,000,000 bps
  last_loop_time = millis();
}

float clampf(float v, float lo, float hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

void loop()
{
  unsigned long time_curr = millis();
  int duty = _DUTY_NEU;
  float a_value = 0.0;
  float dist_raw = 99999.0;
  bool valid_range = false;
  if (time_curr < (last_loop_time + LOOP_INTERVAL)) return;
  last_loop_time += LOOP_INTERVAL;

  a_value = (float)analogRead(PIN_IR);

  if (a_value <= 9.0) {
    dist_raw = 99999.0; 
  } else {
    dist_raw = ((6762.0 / (a_value - 9.0)) - 4.0) * 10.0 - 60.0; // 결과 단위: mm (가정)
  }

  if (dist_raw >= _DIST_MIN && dist_raw <= _DIST_MAX) {
    valid_range = true;
    digitalWrite(PIN_LED, HIGH); // 10~25cm 이내이면 LED ON
  } else {
    valid_range = false;
    digitalWrite(PIN_LED, LOW);
  }

  if (valid_range) {
    dist_ema = (EMA_ALPHA * dist_raw) + ((1.0 - EMA_ALPHA) * dist_ema);
    dist_prev = dist_raw;
  } else {
  }
  // dist_ema가 _DIST_MIN -> _DUTY_MIN, _DIST_MAX -> _DUTY_MAX
  float ratio = (dist_ema - _DIST_MIN) / (_DIST_MAX - _DIST_MIN);
  ratio = clampf(ratio, 0.0, 1.0);            // 0..1로 제한
  duty = (int)(_DUTY_MIN + ratio * (_DUTY_MAX - _DUTY_MIN) + 0.5); 

  myservo.writeMicroseconds(duty);
  Serial.print("_DUTY_MIN:");  Serial.print(_DUTY_MIN);
  Serial.print("_DIST_MIN:");  Serial.print(_DIST_MIN);
  Serial.print(",IR:");        Serial.print(a_value);
  Serial.print(",dist_raw:");  Serial.print(dist_raw);
  Serial.print(",ema:");       Serial.print(dist_ema);
  Serial.print(",servo:");     Serial.print(duty);
  Serial.print(",_DIST_MAX:"); Serial.print(_DIST_MAX);
  Serial.print(",_DUTY_MAX:"); Serial.print(_DUTY_MAX);
  Serial.println("");
}
