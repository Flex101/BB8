#include "Servo.h"
#include <stdio.h>

extern "C" {
	#include "externals/Pico-Servo-main/servo/servo.h"
}

Servo::Servo(uint _pin)
{
	pin = _pin;
	setPosRange(1000,2000);
	setPosHome(1500);
	setServo(pin);
}

void Servo::setPosRange(uint _posMin, uint _posMax)
{
	posMin = _posMin;
	posMax = _posMax;
}

void Servo::setPosHome(uint _posHome)
{
	posHome = _posHome;
}

void Servo::goToMin()
{
	goToPos(posMin);
}

void Servo::goToHome()
{
	goToPos(posHome);
}

void Servo::goToMax()
{
	goToPos(posMax);
}

void Servo::goToPos(uint pos)
{
	if (pos < posMin) pos = posMin;
	if (pos > posMax) pos = posMax;
	setMicroseconds(pin, pos);
}

uint Servo::goToScaledPos(float scaledPos)
{
	uint pos;

	if (scaledPos < 0.0)
	{
		// Negative values are scaled between posMin and posHome
		pos = posHome - ((posHome - posMin) * -scaledPos);
	}
	else
	{
		// Positive values are scaled between posHome and posMax
		pos = posHome + ((posMax - posHome) * scaledPos);
	}

	goToPos(pos);
	return pos;
}

uint Servo::getMin() const
{
	return posMin;
}

uint Servo::getHome() const
{
	return posHome;
}

uint Servo::getMax() const
{
	return posMax;
}
