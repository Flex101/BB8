#pragma once
#include "Common.h"

class Pot
{
public:
	explicit Pot(uint adcNum);
	virtual ~Pot();

	void init(float vRef);
	void init(float vRef, float scaleMin, float scaleMax);
	void setSmoothing(uint samples);
	void update();

	uint16_t getRaw() const;
	float getVoltage() const;
	float getScale() const;

private:
	uint adcNum;
	float vRef;
	float scaleMin;
	float scaleMax;

	uint16_t raw;
	RunningAverage* average;
};