#include "Mapping.h"

using namespace BB8;

BB8::Mapping::Mapping(float _scalar, float _offset) :
	scalar(_scalar), offset(_offset)
{
}

float BB8::Mapping::angleToServo(const float& angle) const
{
	return (angle - offset) / scalar;
}

float BB8::Mapping::servoToAngle(const float& servo) const
{
	return (servo * scalar) + offset;
}
