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
float xAxisDemand = 0;
float yAxisDemand = 0;
float zAxisDemand = 0;
float xAxisActual = 0;
float yAxisActual = 0;

void dataHandler(uint8_t* packet, uint16_t size)
{
	if (size != (3*sizeof(float))) return;
	memcpy(&xAxisDemand, packet, sizeof(float));
	packet += sizeof(float);
	memcpy(&yAxisDemand, packet, sizeof(float));
	packet += sizeof(float);
	memcpy(&zAxisDemand, packet, sizeof(float));
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

	LynxMotionServo yAxisServo(servoPort, 0);
	success &= yAxisServo.init();

	LynxMotionServo xAxisServo(servoPort, 1);
	success &= xAxisServo.init();

	LynxMotionServo zAxisServo(servoPort, 2);
	success &= zAxisServo.init();

	if (!success) abort("Failed to initialise servos");

	MPU6050 imu;
	imu.setPorts(IMU_I2C, IMU_SDA, IMU_SCL);
	imu.init(IMU_DOME_ID);
	imu.calcOffsets(false, false, true, true, true, true);   // Stabilises sensor
	imu.setInclinationOffsets(1.5, 0.8, 0.0);				 // Accounts for error in mounting
	if (!imu.test()) abort("IMU_DOME not found");

	cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
	printf("Initialisation complete!\n");

	xAxisServo.setPosition(0.0);
	zAxisServo.setPosition(0.0);

	while (true)
	{
		imu.update();
		xAxisActual = imu.inclination().x;
		yAxisActual = imu.inclination().y;

		printf("%f %f\n", xAxisActual, yAxisActual);
		sleep_ms(50);
	}

	finish();
}