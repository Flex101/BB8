#include <stdio.h>
#include "DomeMixer.h"
#include "MPU6050.h"
#include "Common.h"
#include <string>

const byte IMU_DOME_ID = 0x69;
const byte IMU_BASE_ID = 0x68;
const uint LED_PIN = 25;

void test(MPU6050& imu, const char* name)
{
	if (!imu.test())
	{
		while (true)
		{
			gpio_put(LED_PIN, 0);
			sleep_ms(500);
			gpio_put(LED_PIN, 1);
			sleep_ms(500);
			printf("%s not found\n", name);
		}
	}
}

int main()
{
	stdio_init_all();

	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);
	gpio_put(LED_PIN, 1);
	
	MPU6050 imuDome;
	imuDome.init(IMU_DOME_ID);
	test(imuDome, "IMU_DOME");

	MPU6050 imuBase;
	imuBase.init(IMU_BASE_ID, false);
	test(imuBase, "IMU_BASE");

	imuDome.calcOffsets(false, false, true, true, true, true);	// Tries to reduce z inclination drift while leaving x and y inclination relative to gravity
	imuBase.calcOffsets(false, false, true, true, true, true);	// Tries to reduce z inclination drift while leaving x and y inclination relative to gravity

	BB8::DomeMixer domeMixer(imuDome, imuBase);
	domeMixer.setDomeToBaseOffsets(-2.5, 1.6, 0); // Fudge offset in mounting

	uint32_t timer = 0;
	while (true)
	{
		domeMixer.update();

		if ((millis() - timer) > 10)
		{
			printf("%f %f %f %f %f %f %f %f %f\n",
				domeMixer.dome().x, domeMixer.dome().y, domeMixer.dome().z, 
				domeMixer.base().x, domeMixer.base().y, domeMixer.base().z,
				domeMixer.domeToBase().x, domeMixer.domeToBase().y, domeMixer.domeToBase().z
			);
			timer = millis();
		}
	}

	return 0;
}
