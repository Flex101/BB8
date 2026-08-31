#include "btController.h"
#include "servoController.h"
#include "tiltController.h"
#include "MPU6050.h"
#include "Common.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/time.h"
#include "hardware/watchdog.h"
#include <stdio.h>

float rollAxisValueBuff = 0.0;
uint32_t lastUpdate_Comms = 0;

void dataHandler(uint8_t* packet, uint16_t size)
{
	if (size != sizeof(float)) return;
	memcpy(&rollAxisValueBuff, packet, sizeof(float));
	lastUpdate_Comms = to_ms_since_boot(get_absolute_time());
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

	if (watchdog_caused_reboot()) {
        printf("WATCHDOG RESET!!!\n");
    }

	sleep_ms(3000);

	if (cyw43_arch_init()) abort("Failed to initialise CYW43 driver.");
	printf("CYW43 driver initialised.\n");

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

	const float imu_angle_offset = 5.0;
	int encoderPos = 0;
	float imu_angle = 0;
	int demandPos = 0;

	sleep_ms(3000);

	uint32_t now = to_ms_since_boot(get_absolute_time());
	uint32_t lastUpdate_Servo = 0;
	uint32_t lastUpdate_Print = 0;
	float rollAxisValue = 0.0;

	watchdog_enable(1000, true);
	while(true)
	{	
		now = to_ms_since_boot(get_absolute_time());
		
		btController.poll();
		rollAxisValue = rollAxisValueBuff;

		if ((now - lastUpdate_Comms) > 1000)
		{
			rollAxisValue = 0.0;
		}

		if ((now - lastUpdate_Servo) > 300)
		{
			servoController.readPos(encoderPos);
			// demandPos += (axisValue * 50);
			// servoController.writePos(demandPos);

			servoController.writeVel(rollAxisValue * 99);

			lastUpdate_Servo = now;
		}

		if ((now - lastUpdate_Print) > 100)
		{
			//printf("Encoder: %d, Angle: %f, Accel: %f\n", encoderPos, imu_angle, imu_acc);
			printf("%d %d %f %d \n", now, lastUpdate_Comms, rollAxisValue, encoderPos);

			lastUpdate_Print = now;
		}

		watchdog_update();
	}

	finish();
	return 0;
}
