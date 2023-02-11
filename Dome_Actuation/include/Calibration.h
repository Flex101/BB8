#pragma once
#include "Common.h"

class Servo;

namespace BB8 
{
	class DomeMixer;

	struct MapEntry
	{
		float demandPos;
		float posDir;
		float negDir;
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