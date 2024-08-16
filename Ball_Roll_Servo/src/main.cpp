#include "btController.h"
#include "servoController.h"
#include "tiltController.h"
#include "MPU6050.h"
#include "Common.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/time.h"
#include <stdio.h>

const byte IMU_ID = 0x68;
const byte IMU_SDA = 14;
const byte IMU_SCL = 15;
float rollAxisValue = 0;
float imu_acc_x = 0;
float imu_acc_y = 0;
float imu_acc_z = 0;
bool sendData = false;

void dataHandler(uint8_t* packet, uint16_t size)
{
	if (size != 4) return;
	memcpy(&rollAxisValue, packet, 4);
	//printf("Axis value: %f\n", axisValue);
}

void dataSender(Byte* packet, uint16_t& size)
{
	if (!sendData)
	{
		size = 0;
		return;
	}

	memcpy(&packet[0], &imu_acc_x, 4);
	memcpy(&packet[4], &imu_acc_y, 4);
	memcpy(&packet[8], &imu_acc_z, 4);
	size = 12;
	sendData = false;
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

	// MPU6050 imu;
	// imu.setPorts(1, IMU_SDA, IMU_SCL);
	// imu.init(IMU_ID);
	// if (!imu.test()) abort("Failed to initialise IMU");
	// // Tries to reduce z inclination drift while leaving x and y inclination relative to gravity
	// imu.calcOffsets(false, false, true, true, true, true);
	// printf("IMU initialised.\n");

	BtController btController("BB-8 Ball Roll", 0xBB8);
	btController.setDataHandler(dataHandler);
	btController.setDataSender(dataSender);
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
	uint32_t lastUpdate_IMU = 0;
	uint32_t lastUpdate_Print = 0;

	while(true)
	{
		btController.poll();
	
		now = to_ms_since_boot(get_absolute_time());
		// if (now - lastUpdate_IMU > 10)
		// {
		// 	imu.update();
		// 	imu_angle = imu.inclination().x + imu_angle_offset;
		// 	imu_acc_x = imu.accel().x;
		// 	imu_acc_y = imu.accel().y;
		// 	imu_acc_z = imu.accel().z;
		// 	sendData = true;

		// 	lastUpdate_IMU = now;
		// }

		if (now - lastUpdate_Servo > 100)
		{
			servoController.readPos(encoderPos);

			// demandPos += (axisValue * 50);
			// servoController.writePos(demandPos);

			servoController.writeVel(rollAxisValue * 99);

			lastUpdate_Servo = now;
		}

		if (now - lastUpdate_Print > 100)
		{
			//printf("Encoder: %d, Angle: %f, Accel: %f\n", encoderPos, imu_angle, imu_acc);
			printf("%f %d %f \n", rollAxisValue, encoderPos);

			lastUpdate_Print = now;
		}
	}
}
