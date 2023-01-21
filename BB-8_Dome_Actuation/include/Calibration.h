#pragma once
#include "Common.h"

class Servo;

namespace BB8 
{
	class DomeMixer;

	enum Direction
	{
		None = 0,
		FB   = 1 << 0,
		LR   = 1 << 1,
		Both = FB & LR
	};

	struct MapEntry
	{
		float demandPos;
		float posDir;
		float negDir;
	};

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

	class Calibration
	{
	public:
		explicit Calibration(Servo& servoFb, Servo& servoLr, DomeMixer& domeMixer);
		virtual ~Calibration() {}

		void mapAxis(Direction direction, uint increments = 10);

	private:
		float getStableValue(Direction direction);

	private:
		Servo& servoFb;
		Servo& servoLr;
		DomeMixer& domeMixer;
	};

} // namespace BB8