#include "MotorDriver.h"

MotorDriver::MotorDriver(uint in1, uint in2) :
	mtr1(in1), mtr2(in2)
{
	
}

void MotorDriver::init()
{
	mtr1.setPosRange(0,20000);
	mtr1.setPosHome(0);
	mtr1.goToPos(0);

	mtr2.setPosRange(0,20000);
	mtr2.setPosHome(0);
	mtr2.goToPos(0);
}

void MotorDriver::setSpeed(float speed)
{
	if (speed > 0)
	{
		mtr1.goToScaledPos(speed);
		mtr2.goToPos(0);
	}
	else
	{
		mtr1.goToPos(0);
		mtr2.goToScaledPos(-speed);
	}
}
