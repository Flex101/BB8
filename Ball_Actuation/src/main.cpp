#include "A4988.h"
#include "Common.h"
#include <string>
#include <math.h>
#include <stdio.h>

const uint LED_PIN = 25;
const uint STP_MS1_PIN = 21;
const uint STP_MS2_PIN = 20;
const uint STP_MS3_PIN = 19;
const uint STP_STP_PIN = 17;
const uint STP_DIR_PIN = 16;

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

	A4988 motorDriver(STP_MS1_PIN, STP_MS2_PIN, STP_MS3_PIN, STP_STP_PIN, STP_DIR_PIN);
	motorDriver.init(A4988::Microsteps::SIXTEENTH);
	motorDriver.setDirection(A4988::Direction::CW);

	sleep_ms(3000);
	while (true)
	{
		motorDriver.step();
		sleep_us(100);
	}

	finish();
	return 0;
}
