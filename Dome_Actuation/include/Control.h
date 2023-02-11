#pragma once
#include "Mapping.h"
#include "Common.h"

class Servo;

namespace BB8
{
	class DomeMixer;

	class Control
	{
	public:
		explicit Control(Servo& servoFb, Servo& servoLr, DomeMixer& domeMixer);
		virtual ~Control() {}

		void spin();

		void setDemand(Direction direction, float angle);
		float getDemand(Direction direction);
		void hold(Direction direction);

		float getCalcDemand(Direction direction);

	private:
		void bangBang(const float& demand, const float& actual, Servo& servo, Mapping& mapping, PID& pid);

	private:
		Servo& servoFb;
		Servo& servoLr;
		DomeMixer& domeMixer;

		Mapping mappingFb;
		Mapping mappingLr;

		float demandFb;
		float demandLr;

		PID pidFb;
		PID pidLr;

		uint32_t prevCalc;
	};

} // namespace BB8
