#include "Control.h"
#include "Servo.h"
#include "DomeMixer.h"
#include "Easings.h"
#include <stdio.h>
#include <string>
#include <math.h>

using namespace BB8;

Control::Control(Servo& _servoFb, Servo& _servoLr, DomeMixer& _domeMixer) :
	servoFb(_servoFb), servoLr(_servoLr), domeMixer(_domeMixer),
	mappingFb(-28.9737026959064,-0.086576333333332),
	mappingLr(36.4420775555555, 1.02449539473684),
	demandFb(0.00f), demandLr(0.00f),
	prevCalc(millis())
{
	const float maxPidOffset =  3.0f;
	pidFb.init(maxPidOffset, -maxPidOffset, 0.01, 0.0, 0.0);
	pidLr.init(maxPidOffset, -maxPidOffset, 0.01, 0.0, 0.0);
}

void Control::spin()
{
	domeMixer.update();

	uint32_t t = millis();
	uint32_t dt = t - prevCalc;

	loop(demandFb, domeMixer.domeToBase().y, servoFb, mappingFb, pidFb, dt);
	loop(demandLr, domeMixer.domeToBase().x, servoLr, mappingLr, pidLr, dt);

	prevCalc = t;
}

void Control::setDemand(Direction direction, float angle)
{
	if (direction == Direction::FB)
	{
		demandFb = angle;
	}
	if (direction == Direction::LR)
	{
		demandLr = angle;
	}
}

float Control::getDemand(Direction direction)
{
	if (direction == Direction::FB) return demandFb;
	if (direction == Direction::LR) return demandLr;
	return 0.00f;
}

void Control::hold(Direction direction)
{
	if (direction == Direction::FB)
		demandFb = domeMixer.domeToBase().y;
	if (direction == Direction::LR)
		demandLr = domeMixer.domeToBase().x;
}

float Control::getCalcDemand(Direction direction)
{
	if (direction == Direction::FB)
		return mappingFb.servoToAngle(servoFb.getScaledPos());
	if (direction == Direction::LR)
		return mappingLr.servoToAngle(servoLr.getScaledPos());
	return 0.0;
}

void Control::loop(const float& demand, const float& actual, Servo& servo, Mapping& mapping, PID& pid, uint32_t dt)
{
	const float inPosTol =       0.1f;
	const float propZone =      30.0f;

	float error = demand - actual;
	float errorMag = fabs(error);

	if (errorMag > propZone) // Bang-bang control
	{
		if (error > 0) servo.goToScaledPos(mapping.angleToServo(90));
		else servo.goToScaledPos(mapping.angleToServo(-90));
	}
	else 
	if (errorMag > inPosTol) // Proportional control
	{		
		float offset = pid.calc(demand, actual, dt);
		servo.goToScaledPos(mapping.angleToServo(demand + offset));
	}
	else // Hold
	{
		servo.goToScaledPos(mapping.angleToServo(demand));
	}
}