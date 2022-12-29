#include <stdio.h>
#include "MPU6050.h"
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

	while (true)
	{
		imu.update();

		printf("%d %d %d\n",
			imu.accel().x, imu.accel().y, imu.accel().z);

		//sleep_ms(100);
	}

	return 0;
}
