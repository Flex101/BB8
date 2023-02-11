#include "servoObject.h"
extern "C" {
#include "servo.h"
}

Servo::Servo(int pin)
{
    pinNumber = pin;
    setServo(pinNumber);
}

void Servo::write(int millis)
{
    setMicroseconds(pinNumber, millis);
}