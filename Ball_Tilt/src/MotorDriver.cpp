#include "MotorDriver.h"

MotorDriver::MotorDriver(uint pwmPin, uint dirPin) :
	pwm(pwmPin), dirPin(dirPin)
{
	
}

void MotorDriver::init()
{
	pwm.setPosRange(0,5000); //20000 MAX - DON'T!
	pwm.setPosHome(0);
	pwm.goToPos(0);

	gpio_init(dirPin);
	gpio_set_dir(dirPin, GPIO_OUT);
	gpio_put(dirPin, 1);
}

void MotorDriver::setSpeed(float speed)
{
	if (abs(speed) < 0.1)
	{
		pwm.goToPos(0);
	}
	else
	{
		pwm.goToScaledPos(abs(speed));
		gpio_put(dirPin, (speed > 0));
	}
}
