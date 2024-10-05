#pragma once
#include <string>

class LynxMotionPort;

class LynxMotionServo
{
public:
	explicit LynxMotionServo(LynxMotionPort& port, int servoId);
	virtual ~LynxMotionServo() {}

	bool init();

private:
	bool checkConnection(int retries = 2);
	bool readPositionRange();

private:
	LynxMotionPort& port;
	int servoId;
};