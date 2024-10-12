#include "LynxMotionServo.h"
#include "LynxMotionPort.h"
#include "Common.h"
#include "pico/stdlib.h"
#include <math.h>
#include <stdio.h>

const bool DEBUG_MSG = false;

LynxMotionServo::LynxMotionServo(LynxMotionPort& port, int servoId) :
	port(port),
	servoId(servoId),
	angularRange(NAN),
	rangeStatus(RangeStatus::UNKNOWN),
	position(NAN),
	enabled(false),
	velocityDemand(0.0f)
{
}

bool LynxMotionServo::init()
{
	bool success = checkConnection();
	disable();

	success &= readAngularRange(angularRange);
	if (DEBUG_MSG) printf("Servo id:%d - Angular Range: %.1f\n", servoId, angularRange);

	return success;
}

bool LynxMotionServo::update()
{
	if (!checkRange()) return false;

	if (enabled)
	{
		if (!enforceAngularRange()) return false;
	}

	return true;
}

void LynxMotionServo::enable()
{
	writeCmd(HALT_HOLD);
	enabled = true;
}

void LynxMotionServo::disable()
{
	writeCmd(LIMP);
	velocityDemand = 0.0f;
	enabled = false;
}

bool LynxMotionServo::setPosition(float angleDeg)
{
	if (!enabled) return false;

	if (angleDeg > angularRange) angleDeg = angularRange;
	if (angleDeg < -angularRange) angleDeg = -angularRange;

	int angleValue = angleDeg * 10;
	writeValue(POSITION_DEG, angleValue);
	return true;
}

bool LynxMotionServo::getPosition(float& angleDeg)
{
	if (isnanf(position)) return false;
	angleDeg = position;
	return true;
}

bool LynxMotionServo::setVelocity(float angleDegSec)
{
	if (!enabled) return false;

	// Trim within servo limits
	if (angleDegSec > MAX_VELOCITY) angleDegSec = MAX_VELOCITY;
	if (angleDegSec < -MAX_VELOCITY) angleDegSec = -MAX_VELOCITY;

	velocityDemand = angleDegSec;
	return true;
}

bool LynxMotionServo::checkConnection(int retries)
{
	std::string msg = "#";
	msg += std::to_string(servoId);
	msg += "QID";
	msg += 0x0D;

	std::string reply;

	std::string expected = "*";
	expected += std::to_string(servoId);
	expected += "QID";
	expected += std::to_string(servoId);
	expected += 0x0D;

	while(retries > 0)
	{
		if (port.send(msg, reply))
		{
			if (reply == expected) return true;
		}
		
		--retries;
	}

	return false;
}

bool LynxMotionServo::checkRange(int retries)
{
	while(retries > 0)
	{
		if(readPosition(position))
		{
			if (position > angularRange) rangeStatus = RangeStatus::PAST_POS_LIMIT;
			else if (position < -angularRange) rangeStatus = RangeStatus::PAST_NEG_LIMIT;
			else if (position > (angularRange - RangeStatus::NEAR_RANGE)) rangeStatus = RangeStatus::NEAR_POS_LIMIT;
			else if (position < (-angularRange + RangeStatus::NEAR_RANGE)) rangeStatus = RangeStatus::NEAR_NEG_LIMIT;
			else rangeStatus = RangeStatus::WITHIN_RANGE;
			return true;
		}

		--retries;
	}

	return false;
}

bool LynxMotionServo::enforceAngularRange()
{
	if (!enabled) return false;

	switch (rangeStatus)
	{
		case RangeStatus::PAST_POS_LIMIT:
			return writeVelocity(-MAX_VELOCITY);

		case RangeStatus::NEAR_POS_LIMIT:
			return writeVelocity(velocityDemand * calcVelocityScale());

		case RangeStatus::NEAR_NEG_LIMIT:
			return writeVelocity(velocityDemand * calcVelocityScale());

		case RangeStatus::PAST_NEG_LIMIT:
			return writeVelocity(MAX_VELOCITY);

		case RangeStatus::WITHIN_RANGE:
			return writeVelocity(velocityDemand);
	}
	
	return true;
}

float LynxMotionServo::calcVelocityScale()
{
	float scale = 1.0f;

	if (rangeStatus == RangeStatus::NEAR_POS_LIMIT)
	{
		if (velocityDemand < 0.0f) scale = 1.0f;
		else scale = (angularRange - position) / RangeStatus::NEAR_RANGE;
	}

	if (rangeStatus == RangeStatus::NEAR_NEG_LIMIT)
	{
		if (velocityDemand > 0.0f) scale = 1.0f;
		else scale = (-angularRange - position) / -RangeStatus::NEAR_RANGE;
	}

	if (scale > 1.0f) scale = 1.0f;
	if (scale < 0.0f) scale = 0.0f;

	return scale;
}

bool LynxMotionServo::readAngularRange(float& result)
{
	int value;
	if (!readValue(ANGULAR_RANGE, value)) return false;
	result = float(value) / 10;
	return true;
}

bool LynxMotionServo::readPosition(float &result)
{
	int value;
	if (!readValue(POSITION_DEG, value)) return false;
	result = float(value) / 10;
	return true;
}

bool LynxMotionServo::writeVelocity(float angleDegSec)
{
	if (!enabled) return false;

	int velocityValue = truncf(angleDegSec);
	writeValue(VELOCITY_DEGSEC, velocityValue);
	return true;
}

bool LynxMotionServo::readValue(std::string variable, int& value)
{
	byte servoId_Byte = std::to_string(servoId)[0];

	std::string msg = "#";
	msg += servoId_Byte;
	msg += "Q";
	msg += variable;
	msg += 0x0D;

	std::string reply;
	bool success = port.send(msg, reply);
	if (!success) return false;

	if (reply[0] != '*') return false;
	if (reply[1] != servoId_Byte) return false;
	if (reply[2] != 'Q') return false;
	if (reply.substr(3, variable.length()) != variable) return false;

	int start = 3 + variable.length();
	int length = reply.length() - (start + 1);
	value = std::stoi(reply.substr(start, length));
	return true;
}

void LynxMotionServo::writeValue(std::string variable, int value)
{
	byte servoId_Byte = std::to_string(servoId)[0];
	std::string valueStr = std::to_string(value);

	std::string msg = "#";
	msg += servoId_Byte;
	msg += variable;
	msg += valueStr;
	msg += 0x0D;

	port.send(msg);
}

void LynxMotionServo::writeCmd(std::string cmd)
{
	byte servoId_Byte = std::to_string(servoId)[0];

	std::string msg = "#";
	msg += servoId_Byte;
	msg += cmd;
	msg += 0x0D;

	port.send(msg);
}