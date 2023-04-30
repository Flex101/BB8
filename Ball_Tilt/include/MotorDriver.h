#pragma once
#include "Servo.h"
#include "Common.h"

class MotorDriver
{
public:
	explicit MotorDriver(uint in1, uint in2);
	virtual ~MotorDriver() {}

	void init();
	void setSpeed(float pos);

private:
	Servo mtr1;
	Servo mtr2;
};