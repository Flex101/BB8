#pragma once
#include <string>

class LynxMotionPort;

class LynxMotionServo
{
public:
	explicit LynxMotionServo(LynxMotionPort& port, int servoId);
	virtual ~LynxMotionServo() {}

	bool init();
	bool setPosition(float angleDeg);

protected:
	bool checkConnection(int retries = 2);
	bool readAngularRange(float& result);
	bool readValue(std::string variable, int& value);
	bool writeValue(std::string variable, int value);

private:
	LynxMotionPort& port;
	int servoId;
	float angularRange;
};