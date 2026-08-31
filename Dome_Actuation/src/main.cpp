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
const byte IMU_BASE_ID = 0x68;
const byte IMU_I2C = 1;
const byte IMU_SDA = 10;
const byte IMU_SCL = 11;

// Front to back axis
float fbDemand = 0.0f;
float fbImuActual = 0.0f;
float fbServoActual = 0.0f;
float fbError = 0.0f;
float fbServoDemand = 0.0f;

// Left to right axis
float lrControlValueBuff = 0.0f;
float lrControlValue = 0.0f;
float lrImuActual = 0.0f;
float lrImuActual_Smooth = 0.0f;
float lrServoDemand = 0.0f;
float lrServoDemand_Smooth = 0.0f;
float lrServoActual = 0.0f;
const float imu2servoScale = 4.0f;
RunningAverage lrImuSmoothing(50);
RunningAverage lrDemandSmoothing(50);

// Spin axis
float spinDemand = 0.0f;
float spinVelocityDemand = 0.0f;


void dataHandler(uint8_t* packet, uint16_t size)
{
	if (size != (3*sizeof(float))) return;
	memcpy(&fbDemand, packet, sizeof(float));
	packet += sizeof(float);
	memcpy(&lrControlValueBuff, packet, sizeof(float));
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
	imu.init(IMU_BASE_ID);
	imu.calcOffsets(false, false, true, true, true, true);   // Stabilises sensor
	imu.setInclinationOffsets(1.5, 0.8, 0.0);				 // Accounts for error in mounting
	if (!imu.test()) abort("IMU_BASE not found");

	cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
	printf("Initialisation complete!\n");

	lrServo.setPositionOffset(20.0f);
	fbServo.setPositionOffset(-30.0f);

	lrServo.enable();
	fbServo.enable();
	spinServo.enable();

	success = lrServo.setPosition(0.0f);
	success &= fbServo.setPosition(0.0f);
	success &= spinServo.setPosition(0.0f);
	if (!success) abort("Failed to home servos!");

	sleep_ms(1000);

	std::string msg = "";
	uint32_t then = 0;
	uint32_t now = 0;
	uint32_t dt = 0;
	uint32_t lastPrint = 0;
	uint32_t lastBlink = 0;
	bool ledState = 1;

	nicePrint("Starting loop...");
	while (true)
	{
		then = now;
		now = to_ms_since_boot(get_absolute_time());
		dt = (now - then);

		imu.update();
		lrImuActual = -imu.inclination().y;
		fbImuActual = imu.inclination().x;

		lrImuSmoothing.append(lrImuActual);

		if (lrServo.update())
		{
		// 	lrError = lrDemand - lrImuActual;
		 	success = lrServo.getPosition(lrServoActual);
			if (success)
			{
				// lrServoDemand = lrImuActual * 4;
				// lrError = abs(lrServoActual - lrServoDemand);
				// if (lrError > 2) success = lrServo.setPosition(lrServoDemand);

				if (lrImuSmoothing.isFull())
				{
					lrImuActual_Smooth = lrImuSmoothing.getAverage();
					lrServoDemand = lrImuActual_Smooth * imu2servoScale;
					lrDemandSmoothing.append(lrServoDemand);
				}

				if (lrDemandSmoothing.isFull())
				{
					lrServoDemand_Smooth = lrDemandSmoothing.getAverage();
					success = lrServo.setPosition(lrServoDemand_Smooth);
				}
			}
			if (!success) msg += "Failed to set lrServo postition";
		}
		else 
		{
			msg += "Failed to update lrServo!";
		}

		if (fbServo.update())
		{
			fbError = fbDemand - fbImuActual;
			success = fbServo.getPosition(fbServoActual);
			if (success)
			{
				fbServoDemand = fbError * MAX_VELOCITY * (float(dt)/1000) * 0.5f;
				//success = fbServo.setPosition(fbServoActual + fbServoDemand);
			}
			if (!success) msg += "Failed to set fbServo postition";
		}
		else 
		{
			msg += "Failed to update fbServo!";
		}

		if (now - lastPrint > 100)
		{
			//printf("%f %f %f %s\n", fbImuActual, fbError, fbServoDemand, msg.c_str());
			printf("%f %f %f %s\n", lrImuActual_Smooth, lrServoDemand_Smooth, lrServoActual, msg.c_str());
			msg =  "";
			lastPrint = now;
		}

		if (now - lastBlink > 100)
		{
			ledState = !ledState;
			cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, ledState);
			lastBlink = now;
		}
	}

	finish();
	return 0;
}