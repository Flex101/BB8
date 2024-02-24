#include "btController.h"
#include "servoController.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/time.h"
#include <stdio.h>

float axisValue;

void dataHandler(uint8_t* packet, uint16_t size)
{
	if (size != 4) return;
	memcpy(&axisValue, packet, 4);
	//printf("Axis value: %f\n", axisValue);
}

int main()
{
	stdio_init_all();
	sleep_ms(3000);

	// initialize CYW43 driver
	if (cyw43_arch_init())
	{
		printf("cyw43_arch_init() failed.\n");
		return -1;
	}

	BtController btController("BB-8 Ball Roll", 0xBB8);
	btController.setDataHandler(dataHandler);

	printf("Delay start...\n");
	sleep_ms(1000);

	ServoController servoController;
	bool success = servoController.init();
	//success &= servoController.setMaxSpeed(99);
	
	if (!success)
	{
		printf("Failed to initialise!\n");

		while (true)
		{
			sleep_ms(250);
			cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
			sleep_ms(250);
			cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
		}
	}

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
