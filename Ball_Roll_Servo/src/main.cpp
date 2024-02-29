#include "btController.h"
#include "servoController.h"
#include "MPU6050.h"
#include "Common.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/time.h"
#include <stdio.h>

const byte IMU_ID = 0x68;
const byte IMU_SDA = 14;
const byte IMU_SCL = 15;
float axisValue;

void dataHandler(uint8_t* packet, uint16_t size)
{
	if (size != 4) return;
	memcpy(&axisValue, packet, 4);
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

	MPU6050 imu;
	imu.setPorts(1, IMU_SDA, IMU_SCL);
	imu.init(IMU_ID);
	if (!imu.test()) abort("Failed to initialise IMU");
	printf("IMU initialised.\n");

	BtController btController("BB-8 Ball Roll", 0xBB8);
	btController.setDataHandler(dataHandler);
	printf("Bluetooth controller initialised.\n");

	printf("Delay start...\n");
	sleep_ms(1000);

	ServoController servoController;
	bool success = servoController.init();
	if (!success) abort("Failed to initialise ServoController");
	printf("ServoController initialised.\n");
	
	cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
	printf("Initialisation complete!\n");

	int encoderPos = 0;
	int demandPos = 0;

	sleep_ms(3000);

	uint32_t lastUpdate = 0;
	uint32_t now = to_ms_since_boot(get_absolute_time());

	while(true)
	{
		btController.poll();

		now = to_ms_since_boot(get_absolute_time());
		if (now - lastUpdate > 100)
		{
			servoController.readPos(encoderPos);
			printf("Encoder: %d\n", encoderPos);

			// demandPos += (axisValue * 50);
			// servoController.writePos(demandPos);

			servoController.writeVel(axisValue * 99);

			lastUpdate = now;
		}
	}
}
