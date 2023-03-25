#pragma once
#include "Common.h"

class Encoder
{
public:
	explicit Encoder(uint pinA, uint pinB);
	virtual ~Encoder() {}

	void init();
	void update();
	void reset();

	int32_t getCount() const;

private:
	uint pinA;
	uint pinB;

	uint A;
	uint B;
	uint prevAB;
	uint currAB;

	int32_t count;
};