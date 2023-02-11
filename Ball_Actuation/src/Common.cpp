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
			if (values[i] < min)
				min = values[i];
			if (values[i] > max)
				max = values[i];
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

PID::PID() :
	initFlag(false), prevError(0.0), integral(0.0)
{
}

void PID::init(double _max, double _min, double _kp, double _kd, double _ki)
{
	max = _max;
	min = _min;
	kp = _kp;
	kd = _kd;
	ki = _ki;
	initFlag = true;
}

bool PID::isInit() const
{
	return initFlag;
}

double PID::calc(double setpoint, double actual, double dt)
{
	// Calculate error
	double error = setpoint - actual;

	// Proportional term
	double Pout = kp * error;

	// Integral term
	integral += error * dt;
	double Iout = ki * integral;

	// Derivative term
	double derivative = (error - prevError) / dt;
	double Dout = kd * derivative;

	// Calculate total output
	double output = Pout + Iout + Dout;

	// Restrict to max/min
	if (output > max)
		output = max;
	else if (output < min)
		output = min;

	// Save error to previous error
	prevError = error;

	return output;
}
