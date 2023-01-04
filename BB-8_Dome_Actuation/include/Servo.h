#pragma once
#include "Common.h"

class Servo
{
public:
	explicit Servo(uint pin);
	virtual ~Servo() {}

	void setPosRange(uint posMin, uint posMax);
	void setPosHome(uint posHome);
	
	void goToMin();
	void goToHome();
	void goToMax();
	void goToPos(uint pos);
	uint goToScaledPos(float scaledPos);

private:
	uint posMin;
	uint posMax;
	uint posHome;
	uint pin;
};