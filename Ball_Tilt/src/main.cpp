#include "MotorDriver.h"
#include "Encoder.h"
#include "Pot.h"
#include "Common.h"
#include <string>
#include <math.h>
#include <stdio.h>

const uint LED_PIN = 25;
const uint MTR_IN1 = 10;
const uint MTR_IN2 = 12;
const uint ENC_A   =  0;
const uint ENC_B   =  1;
const uint POT_ADC =  0;

void abort(const char *msg)
{
	while (true)
	{
		gpio_put(LED_PIN, 0);
		sleep_ms(500);
		gpio_put(LED_PIN, 1);
		sleep_ms(500);
		printf("%s\n", msg);
	}
}

int main()
{
	stdio_init_all();

	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);
	gpio_put(LED_PIN, 1);

	MotorDriver motorDriver(MTR_IN1, MTR_IN2);
	motorDriver.init();

	Encoder encoder(ENC_A, ENC_B);
	encoder.init();

	Pot pot(POT_ADC);
	pot.init(3.3f, 0.011f, 3.299f);
	pot.setSmoothing(50);

	PID pid;
	pid.init(1.00f, -1.00f, 3.00f, 0.00f, 0.00f);

	const uint dur = 3000;
	uint inc = 0;

	float target = 0.0f;
	float actual = 0.0f;
	float demand = 0.0f;

	uint32_t outputTimer = millis();
	uint32_t prevMillis = millis();
	uint32_t dt = float(millis() - prevMillis) / 1000;

	while (true)
	{
		encoder.update();
		pot.update();

		if (pot.isReady())
		{
			actual = (pot.getScale() - 0.5f) * 2;
			dt = float(millis() - prevMillis);
			demand = pid.calc(target, actual, dt);
			motorDriver.setSpeed(-demand);

			// Safety
			if (actual >=  0.91f) motorDriver.setSpeed(0);
			if (actual <= -0.91f) motorDriver.setSpeed(0);

			if ((millis() > outputTimer) && (millis() > 5000))
			{
				if (inc > dur)
				{
					target = 0;
					if (inc > (dur * 3)) inc = 0;
				}				
				else if (inc < (dur/4))
				{
					float prog = float(inc) / (dur/4);
					target = (0.5 + cos(PI + (PI*prog))/2) * 0.9;
				}
				else if (inc > 3*(dur/4))
				{
					float prog = (float(inc) / (dur/4)) - 3;
					target = ((cos(PI + (PI*prog))/2) - 0.5) * 0.9;
				}
				else
				{
					target = sin(2 * PI * (float(inc)/dur)) * 0.9;
				}

				printf("%f\n", target);

				inc++;
				outputTimer = millis();
			}
		}

		//prevMillis = millis();
	}

	finish();
	return 0;
}
