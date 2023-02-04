#include "Common.h"


RunningAverage::RunningAverage(uint _length)
{
	length = _length;
	values = new float[length];

	reset();
}

RunningAverage::~RunningAverage()
{
	delete values;
}

void RunningAverage::append(float value)
{
	values[(index++ % length)] = value;

	if (isFull())
	{
		min = 360.0;
		max = -360.00;
		average = 0.00;

		for (int i = 0; i < length; ++i)
		{
			if (values[i] < min) min = values[i];
			if (values[i] > max) max = values[i];
			average += values[i];
		}

		average = average / length;
		variance = max - min;
	}
}

bool RunningAverage::isFull() const
{
	return (index >= length);
}

void RunningAverage::reset()
{
	index = 0;
}
