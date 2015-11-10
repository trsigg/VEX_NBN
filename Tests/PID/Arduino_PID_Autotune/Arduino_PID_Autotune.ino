#include <Servo.h>
#include <Wire.h>
#include <I2CEncoder.h>
#include <PID_AutoTune_v0.h>
#include <PID_v1.h>

//encoder - gnd to gnd, v to v, scl to analog 5, and sda to analog 4
//motor - gnd to gnd, v to v, control to digital 9

float wheelSpeed;
int motorPower = 135;
long prevMillis, currentMillis;

PID flyWheelPID(&wheelSpeed, &motorPower, 1, 1, 1, DIRECT);
Servo motor;

void setup()
{
  flyWheelPID.SetMode(AUTOMATIC);
  flyWheelPID.SetOutputLimits(0, 180);
  flyWheelPID.SetSampleTime(100);
  
  motor.attach(9);
  
  Wire.begin();
  encoder.init(MOTOR_393_ROTATIONS, MOTOR_393_TIME_DELTA);
  
  encoder.zero();
  prevMillis = millis();
}

void loop()
{
  motor.write(motorPower);
  currentMillis = millis();
  wheelSpeed = encoder.getRawPosition() / (currentMillis - prevMillis);
  encoder.zero();
  prevMillis = currentMillis;
}
