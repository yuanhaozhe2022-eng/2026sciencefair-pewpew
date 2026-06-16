#include <Arduino.h>


int ch1Pin = 2;
int ch2Pin = 3;

int rotIN1 = 5;
int rotIN2 = 6;
int elevIN3 = 9;
int elevIN4 = 10;

int deadzoneMin = 1480;
int deadzoneMax = 1520;
int snapLow     = 1310;
int snapHigh    = 1690;

volatile unsigned long ch1RiseTime = 0;
volatile unsigned long ch2RiseTime = 0;
volatile int ch1Value = 1500;
volatile int ch2Value = 1500;

int ch1Prev = 1500;
int ch2Prev = 1500;

void ch1Interrupt() {
  if (digitalRead(ch1Pin) == HIGH) {
    ch1RiseTime = micros();
  } else {
    ch1Value = micros() - ch1RiseTime;
  }
}

void ch2Interrupt() {
  if (digitalRead(ch2Pin) == HIGH) {
    ch2RiseTime = micros();
  } else {
    ch2Value = micros() - ch2RiseTime;
  }
}

int applyDeadzone(int value, int prevValue) {
  if (value >= deadzoneMin && value <= deadzoneMax) return 1500;

  bool prevInDeadzone = (prevValue >= deadzoneMin && prevValue <= deadzoneMax);
  bool prevWasSnapped = (prevValue == snapLow || prevValue == snapHigh);

  if (value < deadzoneMin && value >= snapLow) {
    if (prevInDeadzone || prevWasSnapped) return snapLow;
  }
  if (value > deadzoneMax && value <= snapHigh) {
    if (prevInDeadzone || prevWasSnapped) return snapHigh;
  }

  return value;
}

void runMotor(int in1, int in2, int pwmInput) {
  int centeredValue = pwmInput - 1500;
  int motorSpeed = map(abs(centeredValue), 0, 500, 0, 255);
  motorSpeed = constrain(motorSpeed, 0, 255);

  if (centeredValue > 0) {
    analogWrite(in1, motorSpeed);
    digitalWrite(in2, LOW);
  } else if (centeredValue < 0) {
    digitalWrite(in1, LOW);
    analogWrite(in2, motorSpeed);
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(rotIN1, OUTPUT);
  pinMode(rotIN2, OUTPUT);
  pinMode(elevIN3, OUTPUT);
  pinMode(elevIN4, OUTPUT);

  digitalWrite(rotIN1, LOW);
  digitalWrite(rotIN2, LOW);
  digitalWrite(elevIN3, LOW);
  digitalWrite(elevIN4, LOW);

  attachInterrupt(digitalPinToInterrupt(ch1Pin), ch1Interrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ch2Pin), ch2Interrupt, CHANGE);

  Serial.println("turret control ready");
}

void loop() {
  noInterrupts();
  int ch1 = ch1Value;
  int ch2 = ch2Value;
  interrupts();

  ch1 = constrain(ch1, 1000, 2000);
  ch2 = constrain(ch2, 1000, 2000);

  ch1 = applyDeadzone(ch1, ch1Prev);
  ch2 = applyDeadzone(ch2, ch2Prev);

  ch1Prev = ch1;
  ch2Prev = ch2;

  runMotor(rotIN1, rotIN2, ch1);
  runMotor(elevIN3, elevIN4, ch2);

  Serial.print("rotation: ");
  Serial.print(ch1);
  Serial.print("  elevation: ");
  Serial.println(ch2);

  delay(20);
}