#pragma once
#include "pico/stdlib.h"

using byte = uint8_t;

const double PI = 3.141592653589793238;

static uint32_t millis()
{
	return to_ms_since_boot(get_absolute_time());
}

struct Frame
{
	float x;
	float y;
	float z;
};

static void finish()
{
	while(true)
	{
		sleep_ms(100);
	}
}

class RunningAverage
{
public:
	explicit RunningAverage(uint length);
	virtual ~RunningAverage();

	void append(float value);
	bool isFull() const;
	void reset();

	float getMin() const        { return min;      }
	float getMax() const        { return max;      }
	float getVariance() const   { return variance; }
	float getAverage() const    { return average;  }

private:
	uint length;
	uint index;
	float* values;
	float min;
	float max;
	float variance;
	float average;
};

namespace BB8
{

	enum Direction
	{
		None = 0,
		FB   = 1 << 0,
		LR   = 1 << 1,
		Both = FB & LR
	};

} // namespace BB8