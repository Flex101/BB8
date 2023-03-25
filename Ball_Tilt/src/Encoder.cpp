#include "Encoder.h"

const int8_t outcome[16] = {0, -1, 1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0};

Encoder::Encoder(uint _pinA, uint _pinB)
{
	pinA = _pinA;
	pinB = _pinB;
	count = 0;
}

void Encoder::init()
{
	gpio_init(pinA);
	gpio_set_dir(pinA, GPIO_IN);
	gpio_init(pinB);
	gpio_set_dir(pinB, GPIO_IN);

	A = gpio_get(pinA);
	B = gpio_get(pinB);
	currAB = (A << 1) | B;
	prevAB = currAB;
}

void Encoder::update()
{
	A = gpio_get(pinA);
	B = gpio_get(pinB);
	currAB = (A << 1) | B;

	uint position = (prevAB << 2) | currAB;
	count += outcome[position];
	prevAB = currAB;
}

void Encoder::reset()
{
	count = 0;
}

int32_t Encoder::getCount() const
{
	return count;
}
