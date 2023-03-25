#include "Servo.h"
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
    
	Servo mtr1(MTR_IN1);
	mtr1.setPosRange(0,20000);
	mtr1.setPosHome(0);
	
	Servo mtr2(MTR_IN2);
	mtr2.setPosRange(0,20000);
	mtr2.setPosHome(0);

	Encoder encoder(ENC_A, ENC_B);
	encoder.init();

	Pot pot(POT_ADC);
	pot.init(3.3f, 0.011f, 3.299f);
	pot.setSmoothing(50);

	const uint dur = 10000;
	uint inc = 0;

	while(true)
	{
		pot.update();
		printf("%d %f %f\n", pot.getRaw(), pot.getVoltage(), pot.getScale());
	}

	uint32_t outputTimer = millis();
	while (true)
	{
		encoder.update();

		if ((millis() > outputTimer))
		{
			float demand = sin(2 * PI * (float(inc)/dur));

			if (demand > 0)
			{
				mtr1.goToScaledPos(demand);
				mtr2.goToScaledPos(0);
			}
			else
			{
				mtr1.goToScaledPos(0);
				mtr2.goToScaledPos(-demand);
			}

			printf("%f %d\n", demand * 2500, encoder.getCount());

			inc++;
			outputTimer = millis();
		}
	}

	finish();
	return 0;
}
