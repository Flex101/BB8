#include <stdio.h>
#include "MPU6050.h"
#include "Common.h"
#include "pico/stdlib.h"

int main()
{
	stdio_init_all();
	sleep_ms(3000);

	const byte IMU_FB_ID = 0x69;
	const byte IMU_LR_ID = 0x68;
	const uint LED_PIN = 25;

	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);
	gpio_put(LED_PIN, 1);
	MPU6050 imu_fb;
	imu_fb.init(IMU_FB_ID);
	if (!imu_fb.test())
	{
		while (true)
		{
			gpio_put(LED_PIN, 0);
			sleep_ms(500);
			gpio_put(LED_PIN, 1);
			sleep_ms(500);
			printf("IMU_FB not found\n");
		}
	}

	MPU6050 imu_lr;
	imu_lr.init(IMU_LR_ID, false);
	if (!imu_lr.test())
	{
		while (true)
		{
			gpio_put(LED_PIN, 0);
			sleep_ms(500);
			gpio_put(LED_PIN, 1);
			sleep_ms(500);
			printf("IMU_LR not found\n");
		}
	}

	uint32_t timer = 0;
	imu_fb.calcGyroOffsets();	// Tries to reduce z inclination drift while leaving x and y inclination relative to gravity
	imu_lr.calcGyroOffsets();	// Tries to reduce z inclination drift while leaving x and y inclination relative to gravity

	while (true)
	{
		imu_fb.update();
		imu_lr.update();

		if ((millis() - timer) > 10)
		{
			printf("%f %f %f %f %f %f\n", imu_fb.inclination().x, imu_fb.inclination().y, imu_fb.inclination().z, imu_lr.inclination().x, imu_lr.inclination().y, imu_lr.inclination().z);
			timer = millis();
		}
	}

	return 0;
}
