#include "Pot.h"

Pot::Pot(uint _adcNum)
{
	adcNum = _adcNum;
	average = nullptr;
}

Pot::~Pot()
{
	delete average;
}

void Pot::init(float _vRef)
{
	vRef = _vRef;
	scaleMin = 0;
	scaleMax = vRef;
}

void Pot::init(float _vRef, float _scaleMin, float _scaleMax)
{
	adc_init();
	adc_gpio_init(26 + adcNum);
	adc_select_input(adcNum);

	vRef = _vRef;
	scaleMin = _scaleMin;
	scaleMax = _scaleMax;
}

void Pot::setSmoothing(uint samples)
{
	average = new RunningAverage(samples);
}

void Pot::update()
{
	raw = adc_read();
	if (average != nullptr) average->append(raw);
}

uint16_t Pot::getRaw() const
{
	return raw;
}

float Pot::getVoltage() const
{
	if (average != nullptr && average->isFull())
	{
		return average->getAverage() * (vRef / ((1 << 12) - 1));
	}
	else
	{
		return raw * (vRef / ((1 << 12) - 1));
	}
}

float Pot::getScale() const
{
	float voltage = getVoltage();

	if (voltage < scaleMin) return 0.0f;
	if (voltage > scaleMax) return 1.0f;
	return (voltage - scaleMin) / (scaleMax - scaleMin);
}
