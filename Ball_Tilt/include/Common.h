#pragma once
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/structs/sio.h"
#include "hardware/adc.h"
#include "math.h"

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

static float matchSign(const float& value, const float& matchTo)
{
	return value * (matchTo / fabs(matchTo));
}

class PID
{
public:
	// max - maximum value of manipulated variable
	// min - minimum value of manipulated variable
	// kp  - proportional gain
	// kd  - derivative gain
	// ki  - integral gain
	explicit PID();
	virtual ~PID() {}

	void init(double max, double min, double kp, double kd, double ki);
	bool isInit() const;

	// setpoint - demand value
	// actual   - actual value
	// dt       - delta time since last call
	double calc(double setpoint, double actual, double dt);

private:
	bool initFlag;
	double max;
	double min;
	double kp;
	double kd;
	double ki;
	double prevError;
	double integral;
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