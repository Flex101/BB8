#pragma once
#include "math.h"

class Easings
{
public:
	static double linear(double prog)
	{
		return prog;
	}

	static double easeOutQuart(double prog)
	{
		return 1 - pow((1 - prog), 4);
	}

	static double easeInQuart(double prog)
	{
		return pow(prog, 4);
	}
};