#pragma once
#include "Servo.h"
#include "Common.h"

class MotorDriver
{
public:
	explicit MotorDriver(uint pwmPin, uint dirPin);
	virtual ~MotorDriver() {}

	void init();
	void setSpeed(float pos);

private:
	Servo pwm;
	uint dirPin;
};