#include "A4988.h"

A4988::A4988(const uint& _pinMs1, const uint& _pinMs2, const uint& _pinMs3, const uint& _pinStp, const uint& _pinDir) :
	pinMs1(_pinMs1), pinMs2(_pinMs2), pinMs3(_pinMs3), pinStp(_pinStp), pinDir(_pinDir)
{
}

void A4988::init(const Microsteps& microsteps)
{
	gpio_init(pinMs1);
	gpio_set_dir(pinMs1, GPIO_OUT);
	gpio_init(pinMs2);
	gpio_set_dir(pinMs2, GPIO_OUT);
	gpio_init(pinMs3);
	gpio_set_dir(pinMs3, GPIO_OUT);
	gpio_init(pinStp);
	gpio_set_dir(pinStp, GPIO_OUT);
	gpio_init(pinDir);
	gpio_set_dir(pinDir, GPIO_OUT);

	gpio_put(pinMs1, microsteps & 0b100);
	gpio_put(pinMs2, microsteps & 0b010);
	gpio_put(pinMs3, microsteps & 0b001);
}

void A4988::setDirection(const Direction& dir)
{
	gpio_put(pinDir, dir);
}

void A4988::step(const uint &steps)
{
	for (uint i = 0; i < steps; ++i)
	{
		gpio_put(pinStp, 1);
		sleep_us(30);
		gpio_put(pinStp, 0);
		sleep_us(30);
	}
}
