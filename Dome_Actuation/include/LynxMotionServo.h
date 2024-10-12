#pragma once
#include "LynxMotionVars.h"
#include <string>

class LynxMotionPort;

class LynxMotionServo
{
public:
	explicit LynxMotionServo(LynxMotionPort& port, int servoId);
	virtual ~LynxMotionServo() {}

	bool init();
	bool update();

	void enable();
	void disable();

	bool setPosition(float angleDeg);
	bool getPosition(float& angleDeg);
	bool setVelocity(float angleDegSec);

protected:
	bool checkConnection(int retries = 2);
	bool checkRange(int retries = 2);
	bool enforceAngularRange();
	float calcVelocityScale();
	bool readAngularRange(float& result);
	bool readPosition(float& result);
	bool writeVelocity(float angleDegSec);

private:
	bool readValue(std::string variable, int& value);
	void writeValue(std::string variable, int value);
	void writeCmd(std::string cmd);

private:
	LynxMotionPort& port;
	int servoId;
	float angularRange;
	RangeStatus::Enum rangeStatus;
	float position;
	bool enabled;

	float velocityDemand;
};