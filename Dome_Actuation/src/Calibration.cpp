#include "Calibration.h"
#include "Servo.h"
#include "DomeMixer.h"
#include <stdio.h>
#include <string>

using namespace BB8;

Calibration::Calibration(Servo& _servoFb, Servo& _servoLr, DomeMixer& _domeMixer) :
	servoFb(_servoFb), servoLr(_servoLr), domeMixer(_domeMixer)
{

}

void Calibration::mapAxis(Direction direction, uint increments)
{
	std::string axis = "";
	if (direction == Direction::FB) axis = "FB";
	else if (direction == Direction::LR) axis = "LR";
	printf("Mapping %s...\n", axis.c_str());
	
	float stepSize = 1.00 / increments;
	float step = 0.00;
	float value = 0.00;

	Servo* servo = nullptr;
	if (direction == Direction::FB) servo = &servoFb;
	else if (direction == Direction::LR) servo = &servoLr;

	MapEntry map[(increments * 2) + 1];
	for (int i = 0; i < (increments * 2) + 1; ++i)
	{
		map[i].demandPos = -1.00 + (stepSize * i);
		map[i].posDir =  0.0;
		map[i].negDir =  0.0;
	}

	// Positive value - Positive direction
	for (int i = 1; i <= increments; ++i)
	{
		step = stepSize * i;
		printf("Moving %s to +%f\n", axis.c_str(), step);
		servo->goToScaledPos(step);
		value = getStableValue(direction);

		map[increments + i].posDir = value;
	}

	// Positive value - Negative direction
	for (int i = 1; i <= increments; ++i)
	{
		step = 1.00 - (stepSize * i);
		printf("Moving %s to +%f\n", axis.c_str(), step);
		servo->goToScaledPos(step);
		value = getStableValue(direction);

		map[(2 * increments) - i].negDir = value;
	}

	// Negative value - Negative direction
	for (int i = 1; i <= increments; ++i)
	{
		step = -(stepSize * i);
		printf("Moving %s to %f\n", axis.c_str(), step);
		servo->goToScaledPos(step);
		value = getStableValue(direction);

		map[increments - i].negDir = value;
	}

	// Negative value - Position direction
	for (int i = 1; i <= increments; ++i)
	{
		step = -1.00 + (stepSize * i);
		printf("Moving %s to %f\n", axis.c_str(), step);
		servo->goToScaledPos(step);
		value = getStableValue(direction);

		map[i].posDir = value;
	}

	printf("Mapping complete!\n");

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < (increments * 2) + 1; ++j)
		{
			switch (i)
			{
				case 0:
					printf("%f\t", map[j].demandPos);
					break;

				case 1:
					printf("%f\t", map[j].posDir);
					break;

				case 2:
					printf("%f\t", map[j].negDir);
					break;
			}
		}

		printf("\n");
	}
}

float Calibration::getStableValue(Direction direction)
{
	/*
	printf("Waiting for ");
	if (direction == Direction::FB) printf("FB ");
	else if (direction == Direction::LR) printf("LR ");
	printf("to stabilise...\n");
	*/

	const float stableTolerance = 0.2;
	const uint stableDuration_ms = 1000;
	const uint stableInterval_ms = 100;
	const uint count = stableDuration_ms / stableInterval_ms;

	RunningAverage average(count);

	while(true)
	{
		domeMixer.update();

		if (direction == Direction::FB) average.append(domeMixer.domeToBase().y);
		else if (direction == Direction::LR) average.append(domeMixer.domeToBase().x);

		if (average.isFull())
		{
			if (average.getVariance() < stableTolerance) return average.getAverage();
		}

		sleep_ms(stableInterval_ms);
	}
}