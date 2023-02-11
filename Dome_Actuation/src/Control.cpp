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

}

void Control::spin()
{
	domeMixer.update();

	//bangBang(demandFb, domeMixer.domeToBase().y, servoFb, mappingFb);
	bangBang(demandLr, domeMixer.domeToBase().x, servoLr, mappingLr, pidLr);
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

void Control::bangBang(const float& demand, const float& actual, Servo& servo, Mapping& mapping, PID& pid)
{
	const float inPosTol =       0.1f;
	const float propZone =      20.0f;
	const float maxPropOffset =  3.0f;
	const float deadZone =       2.25f;

	if (!pid.isInit())
	{
		pid.init(maxPropOffset, -maxPropOffset, 0.65, 5.0, 0.0);
	}

	uint32_t t = millis();
	uint32_t dt = t - prevCalc;
	float error = demand - actual;
	float errorMag = fabs(error);

	// if (errorMag > propZone) // Bang-bang control
	// {
	// 	if (error > 0) servo.goToScaledPos(1.00f);
	// 	else servo.goToScaledPos(-1.00f);
	// }
	// else 
	// if (errorMag > inPosTol) // Proportional control
	// {
		
		float offset = pid.calc(demand, actual, dt);

		//float range = (error / propZone);
		//float offset = (Easings::linear(range) * maxPropOffset);
		//if (fabs(offset) < deadZone) offset = matchSign(deadZone, offset);

		servo.goToScaledPos(mapping.angleToServo(demand + offset));
	// }
	// else // Hold
	// {
	// 	servo.goToScaledPos(mapping.angleToServo(demand));
	// }

	prevCalc = t;
}