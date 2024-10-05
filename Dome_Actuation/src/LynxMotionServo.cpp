#include "LynxMotionServo.h"
#include "LynxMotionPort.h"
#include "Common.h"
#include "pico/stdlib.h"
#include <stdio.h>

LynxMotionServo::LynxMotionServo(LynxMotionPort& port, int servoId) :
	port(port), servoId(servoId)
{
}

bool LynxMotionServo::init()
{
	bool success = checkConnection();

	return success;
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

bool LynxMotionServo::readPositionRange()
{
	return false;
}