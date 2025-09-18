const int ledPin = 7;
int count = 0;

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, 1);
  delay(1000);
  digitalWrite(ledPin, 0);
}

void loop() {
  while (count < 5) {
    digitalWrite(ledPin, 0);
    delay(100);
    digitalWrite(ledPin, 1);
    delay(100);
    count += 1;     
  }
  digitalWrite(ledPin, 0);
  while (1) {
    //
  }
}
