#pragma once
#include "pico/stdlib.h"

class A4988
{
public:
	enum Microsteps
	{
		FULL      = 0b000,
		HALF      = 0b100,
		QUARTER   = 0b010,
		EIGTH     = 0b110,
		SIXTEENTH = 0b111
	};

	enum Direction
	{
		CCW = 0,
		CW = 1
	};

public:
	explicit A4988(const uint& pinMs1, const uint& pinMs2, const uint& pinMs3, const uint& pinStp, const uint& pinDir);
	virtual ~A4988() {}

	void init(const Microsteps& microsteps = FULL);

	void setDirection(const Direction& dir);
	void step(const uint& steps = 1);

private:
	const uint pinMs1;
	const uint pinMs2;
	const uint pinMs3;
	const uint pinStp;
	const uint pinDir;
};