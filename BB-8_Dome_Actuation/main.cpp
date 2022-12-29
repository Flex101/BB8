#include <stdio.h>
#include "MPU6050.h"
#include "Common.h"
#include "pico/stdlib.h"

int main()
{
	stdio_init_all();

	const uint LED_PIN = 25;
	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);
	gpio_put(LED_PIN, 1);

	MPU6050 imu;
	imu.init();
	if (!imu.test())
	{
		while (true)
		{
			gpio_put(LED_PIN, 0);
			sleep_ms(500);
			gpio_put(LED_PIN, 1);
			sleep_ms(500);
		}
	}

	uint32_t timer = 0;
	imu.calcGyroOffsets();	// Tries to reduce z inclination drift while leaving x and y inclination absolute

	while (true)
	{
		imu.update();

		if ((millis() - timer) > 10)
		{
			printf("%f %f %f\n", imu.inclination().x, imu.inclination().y, imu.inclination().z);
			timer = millis();
		}
	}

	return 0;
}
