#include "Servo.h"

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

void Servo::setPos(uint pos)
{
	if (pos < posMin) pos = posMin;
	if (pos > posMax) pos = posMax;
	setMicroseconds(pin, pos);
}

void Servo::min()
{
	setPos(posMin);
}

void Servo::home()
{
	setPos(posHome);
}

void Servo::max()
{
	setPos(posMax);
}
