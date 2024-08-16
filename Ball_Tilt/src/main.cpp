#include "btController.h"
#include "MotorDriver.h"
#include "Encoder.h"
#include "Pot.h"
#include "DemandInput.h"
#include "Common.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include <string>
#include <math.h>
#include <stdio.h>

const uint LED_PIN = 25;
const uint MTR_PWM = 21;
const uint MTR_DIR = 20;
const uint ENC_A   =  0;
const uint ENC_B   =  1;
const uint POT_ADC =  0;
float tiltAxisValue = 0;

void dataHandler(uint8_t* packet, uint16_t size)
{
	if (size != 4) return;
	memcpy(&tiltAxisValue, packet, 4);
	//printf("Axis value: %f\n", axisValue);
}

void abort(const std::string& msg)
{
	printf("%s\n", msg.c_str());

	while (true)
	{
		sleep_ms(250);
		cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
		sleep_ms(250);
		cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
	}
}
int main()
{
	stdio_init_all();
	sleep_ms(3000);

	if (cyw43_arch_init()) abort("Failed to initialise CYW43 driver.");
	printf("CYW43 driver initialised.\n");

	BtController btController("BB-8 Ball Tilt", 0xBB8);
	btController.setDataHandler(dataHandler);
	printf("Bluetooth controller initialised.\n");

	printf("Delay start...\n");
	sleep_ms(1000);

	MotorDriver motorDriver(MTR_PWM, MTR_DIR);
	motorDriver.init();

	Encoder encoder(ENC_A, ENC_B);
	encoder.init();

	Pot pot(POT_ADC);
	pot.init(3.3f, 0.011f, 3.299f);
	pot.setSmoothing(75);

	PID pid;
	pid.init(1.00f, -1.00f, 3.00f, 0.00f, 0.00f);

	DemandInput demandInput;
	demandInput.init();

	cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
	printf("Initialisation complete!\n");

	const uint dur = 3000;
	uint inc = 0;

	float target = 0.0f;
	float actual = 0.0f;
	float demand = 0.0f;

	uint32_t outputTimer = millis();
	uint32_t prevMillis = millis();
	uint32_t dt = float(millis() - prevMillis) / 1000;

	sleep_ms(3000);

	uint32_t now = to_ms_since_boot(get_absolute_time());
	uint32_t lastUpdate_Print = 0;
	uint32_t lastUpdate_Motor = 0;

	while (true)
	{
		now = to_ms_since_boot(get_absolute_time());

		btController.poll();
		encoder.update();
		pot.update();

		target = tiltAxisValue;
		if (target >  0.5) target =  0.5;
		if (target < -0.5) target = -0.5;

		if (pot.isReady())
		{
			actual = (pot.getScale() - 0.5f) * 2;
			dt = float(millis() - prevMillis);
			demand = pid.calc(target, actual, dt);
			motorDriver.setSpeed(-demand);

			// Safety
			if (actual >=  0.91f) motorDriver.setSpeed(0);
			if (actual <= -0.91f) motorDriver.setSpeed(0);

			//prevMillis = millis();
		}

		if (now - lastUpdate_Print > 100)
		{
			printf("%f %f\n", tiltAxisValue, actual);
		}
	}

	finish();
	return 0;
}
