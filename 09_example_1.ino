// Arduino pin assignment
#include <Arduino.h>
#define PIN_LED  10
#define PIN_TRIG 12
#define PIN_ECHO 13

// configurable parameters
#define SND_VEL 346.0     // sound velocity at 24 celsius degree (unit: m/sec)
#define INTERVAL 25       // sampling interval (unit: msec)
#define PULSE_DURATION 10 // ultra-sound Pulse Duration (unit: usec)
#define _DIST_MIN 100     // minimum distance to be measured (unit: mm)
#define _DIST_MAX 300     // maximum distance to be measured (unit: mm)

#define TIMEOUT ((INTERVAL / 2) * 1000.0) // maximum echo waiting time (unit: usec)
#define SCALE (0.001 * 0.5 * SND_VEL)     // coefficent to convert duration to distance

#define _EMA_ALPHA 0.10f    // EMA weight of new sample (range: 0 to 1)
                          // Setting EMA to 1 effectively disables EMA filter.

// global variables
constexpr int SAMPLE_SIZE = 30;      //sample size (샘플 크기)
unsigned long last_sampling_time;   // unit: msec
float dist_prev = _DIST_MAX;        // Distance last-measured
float dist_ema;                     // EMA distance
float dist_med = 0.0f;
int counter = 0;

// circular buffer (fixed size)
float sampleArray[SAMPLE_SIZE] = {0.0f};

///// Utility: median for small arrays (copies to tmp and insertion sorts, 중위수 구하기)
float median_of_array(const float *arr, int n) {
  if (n <= 0) return 0.0f;
  // copy to tmp
  float tmp[ SAMPLE_SIZE ]; // max SAMPLE_SIZE
  for (int i = 0; i < n; ++i) tmp[i] = arr[i];

  // insertion sort (efficient for small n)
  for (int i = 1; i < n; ++i) {
    float key = tmp[i];
    int j = i - 1;
    while (j >= 0 && tmp[j] > key) {
      tmp[j + 1] = tmp[j];
      --j;
    }
    tmp[j + 1] = key;
  }

  if (n % 2 == 1) return tmp[n / 2];
  return (tmp[n / 2 - 1] + tmp[n / 2]) / 2.0f;
}

void setup() {
  // initialize GPIO pins
  pinMode(PIN_LED,OUTPUT);
  pinMode(PIN_TRIG,OUTPUT);
  pinMode(PIN_ECHO,INPUT);
  digitalWrite(PIN_TRIG, LOW);

  // initialize serial port
  Serial.begin(57600);
}

void loop() {
  float dist_raw, dist_filtered;
  
  // wait until next sampling time. 
  // millis() returns the number of milliseconds since the program started. 
  // will overflow after 50 days.
  if (millis() < last_sampling_time + INTERVAL)
    return;

  // get a distance reading from the USS
  dist_raw = USS_measure(PIN_TRIG,PIN_ECHO);

  
  if (dist_raw == 0.0f || dist_raw > _DIST_MAX || dist_raw < _DIST_MIN) {
    dist_filtered = dist_prev;
  } else {
    dist_filtered = dist_raw;
    dist_prev = dist_raw;
  }

    if (counter < SAMPLE_SIZE) {
    sampleArray[counter] = dist_raw;
    // compute median of the filled portion (counter+1)
    dist_med = median_of_array(sampleArray, counter + 1);
  } else {
    // circular overwrite
    sampleArray[counter % SAMPLE_SIZE] = dist_raw;
    dist_med = median_of_array(sampleArray, SAMPLE_SIZE);
  }


  // do something here
  if ((dist_raw < _DIST_MIN) || (dist_raw > _DIST_MAX))
    digitalWrite(PIN_LED, 1);       // LED OFF
  else
    digitalWrite(PIN_LED, 0);       // LED ON

  // update last sampling time
  last_sampling_time += INTERVAL;
  ++counter;
  //신규 시리얼 출력 방식
  Serial.print("Min:"); Serial.print(_DIST_MIN);
  Serial.print(",raw:"); Serial.print(dist_raw);
  Serial.print(",ema:"); Serial.print(dist_ema);
  Serial.print(",median:"); Serial.print(dist_med);
  Serial.print(",Max:"); Serial.print(_DIST_MAX);
  Serial.println();
}

// get a distance reading from USS. return value is in millimeter.
float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE; // unit: mm

  // Pulse duration to distance conversion example (target distance = 17.3m)
  // - pulseIn(ECHO, HIGH, timeout) returns microseconds (음파의 왕복 시간)
  // - 편도 거리 = (pulseIn() / 1,000,000) * SND_VEL / 2 (미터 단위)
  //   mm 단위로 하려면 * 1,000이 필요 ==>  SCALE = 0.001 * 0.5 * SND_VEL
  //
  // - 예, pusseIn()이 100,000 이면 (= 0.1초, 왕복 거리 34.6m)
  //        = 100,000 micro*sec * 0.001 milli/micro * 0.5 * 346 meter/sec
  //        = 100,000 * 0.001 * 0.5 * 346
  //        = 17,300 mm  ==> 17.3m
}
