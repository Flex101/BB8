#include "LynxMotionServo.h"
#include "LynxMotionPort.h"
#include "LynxMotionVars.h"
#include "Common.h"
#include "pico/stdlib.h"
#include <stdio.h>

LynxMotionServo::LynxMotionServo(LynxMotionPort& port, int servoId) :
	port(port), servoId(servoId), angularRange(180.0)
{
}

bool LynxMotionServo::init()
{
	bool success = checkConnection();
	success &= readAngularRange(angularRange);

	printf("Angular Range: %f\n", angularRange);

	return success;
}

bool LynxMotionServo::setPosition(float angleDeg)
{
	int angleValue = angleDeg * 10;
	return writeValue(POSITION_DEG, angleValue);
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
		bool success = port.send(msg, reply);
		if (!success) return false;

		if (reply == expected) return true;
		--retries;
	}

	return false;
}

bool LynxMotionServo::readAngularRange(float& result)
{
	int value;
	if (!readValue(ANGULAR_RANGE, value)) return false;
	result = float(value) / 10;
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

bool LynxMotionServo::writeValue(std::string variable, int value)
{
	byte servoId_Byte = std::to_string(servoId)[0];
	std::string valueStr = std::to_string(value);

	std::string msg = "#";
	msg += servoId_Byte;
	msg += variable;
	msg += valueStr;
	msg += 0x0D;

	std::string reply;
	bool success = port.send(msg, reply);
	if (!success) return false;

	if (reply[0] != '*') return false;
	if (reply[1] != servoId_Byte) return false;
	if (reply.substr(2, variable.length()) != variable) return false;
	if (reply.substr(2 + variable.length(), valueStr.length()) != valueStr) return false;
	return true;
}
