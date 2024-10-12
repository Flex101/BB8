#include "btController.h"
#include "LynxMotionPort.h"
#include "LynxMotionServo.h"
#include "MPU6050.h"
#include "Common.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/time.h"

uart_inst_t* UART_ID = uart0;
const uint8_t UART_TX_PIN = 16;
const uint8_t UART_RX_PIN = 17;
const byte IMU_DOME_ID = 0x69;
const byte IMU_I2C = 1;
const byte IMU_SDA = 10;
const byte IMU_SCL = 11;
float fbDemand = 0;
float lrDemand = 0;
float spinDemand = 0;
float fbActual = 0;
float lrActual = 0;

void dataHandler(uint8_t* packet, uint16_t size)
{
	if (size != (3*sizeof(float))) return;
	memcpy(&fbDemand, packet, sizeof(float));
	packet += sizeof(float);
	memcpy(&lrDemand, packet, sizeof(float));
	packet += sizeof(float);
	memcpy(&spinDemand, packet, sizeof(float));
	//printf("Axis value: %f\n", axisValue);
}

void abort(const std::string& msg = "")
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

	BtController btController("BB-8 Dome", 0xBB8);
	btController.setDataHandler(dataHandler);
	printf("Bluetooth controller initialised.\n");

	printf("Delay start...\n");
	sleep_ms(1000);

	LynxMotionPort servoPort(UART_ID, UART_TX_PIN, UART_RX_PIN);
	bool success = servoPort.init();

	LynxMotionServo fbServo(servoPort, 0);
	success &= fbServo.init();

	LynxMotionServo lrServo(servoPort, 1);
	success &= lrServo.init();

	LynxMotionServo spinServo(servoPort, 2);
	success &= spinServo.init();

	if (!success) abort("Failed to initialise servos");

	MPU6050 imu;
	imu.setPorts(IMU_I2C, IMU_SDA, IMU_SCL);
	imu.init(IMU_DOME_ID);
	imu.calcOffsets(false, false, true, true, true, true);   // Stabilises sensor
	imu.setInclinationOffsets(1.5, 0.8, 0.0);				 // Accounts for error in mounting
	if (!imu.test()) abort("IMU_DOME not found");

	cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
	printf("Initialisation complete!\n");

	lrServo.enable();
	fbServo.enable();
	spinServo.enable();

	success = lrServo.setPosition(0.0);
	success &= fbServo.setPosition(-30.0);
	success &= spinServo.setVelocity(-90.0f);
	if (!success) abort("Failed to home servos!");

	nicePrint("Starting loop...");
	while (true)
	{
		imu.update();
		fbActual = imu.inclination().x;
		lrActual = imu.inclination().y;

		float servoPos = NAN;
		success = spinServo.update();
		success &= spinServo.getPosition(servoPos);
		if (!success) printf("Failed to update servo!\n");		

		printf("%f %f %.1f\n", fbActual, lrActual, servoPos);
		sleep_ms(1);
	}

	finish();
}