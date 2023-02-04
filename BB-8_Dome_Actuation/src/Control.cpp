#include "Control.h"
#include "Servo.h"
#include "DomeMixer.h"
#include <stdio.h>
#include <string>
#include <math.h>

using namespace BB8;

Control::Control(Servo& _servoFb, Servo& _servoLr, DomeMixer& _domeMixer) :
	servoFb(_servoFb), servoLr(_servoLr), domeMixer(_domeMixer),
	mappingFb(-28.9737026959064,-0.086576333333332),
	mappingLr(36.4420775555555, 1.02449539473684),
	demandFb(0.00f), demandLr(0.00f),
	holdingFb(false), holdingLr(false)
{

}

void Control::spin()
{
	domeMixer.update();

	//bangBang(demandFb, domeMixer.domeToBase().y, servoFb, mappingFb);
	bangBang(demandLr, domeMixer.domeToBase().x, servoLr, mappingLr, holdingLr);
}

void Control::setDemand(Direction direction, float angle)
{
	if (direction == Direction::FB)
	{
		demandFb = angle;
		holdingFb = false;
	}
	if (direction == Direction::LR)
	{
		demandLr = angle;
		holdingLr = false;
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

void Control::bangBang(const float& demand, const float& actual, Servo& servo, Mapping& mapping, bool& holding)
{
	const float inPosTol =       5.0f;
	const float propZone =      20.0f;
	const float maxPropOffset =  6.0f;

	float error = demand - actual;
	float errorMag = fabs(error);

	if (errorMag > propZone) // Bang-bang control
	{
		if (error > 0) servo.goToScaledPos(1.00f);
		else servo.goToScaledPos(-1.00f);
	}
	else if (errorMag > inPosTol) // Proportional control
	{
		float offset = (error / propZone) * maxPropOffset;
		servo.goToScaledPos(mapping.angleToServo(demand + offset));
	}
	else // Hold
	{
		servo.goToScaledPos(mapping.angleToServo(demand));
	}
}
