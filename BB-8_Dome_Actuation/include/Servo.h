#pragma once
#include "Common.h"

class Servo
{
public:
	explicit Servo(uint pin);
	virtual ~Servo() {}

	void setPosRange(uint posMin, uint posMax);
	void setPosHome(uint posHome);
	void setPos(uint pos);

	void min();
	void home();
	void max();

private:
	uint posMin;
	uint posMax;
	uint posHome;
	uint pin;
};