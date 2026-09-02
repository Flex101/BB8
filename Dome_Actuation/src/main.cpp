#include "btController.h"
#include "LynxMotionPort.h"
#include "LynxMotionServo.h"
#include "MPU6050.h"
#include "Common.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/time.h"

const int cm_disabled = 0;	// 0 = disabled
const int cm_auto = 1;		// 1 = auto
const int cm_manual = 2;	// 2 = manual
int controlModeBuff = 0;
int controlMode = 0;

uart_inst_t* UART_ID = uart0;
const uint8_t UART_TX_PIN = 16;
const uint8_t UART_RX_PIN = 17;
const byte IMU_DOME_ID = 0x69;
const byte IMU_BASE_ID = 0x68;
const byte IMU_I2C = 1;
const byte IMU_SDA = 10;
const byte IMU_SCL = 11;

// Front to back axis
float fbControlValueBuff = 0.0f;
float fbControlValue = 0.0f;
float fbImuActual = 0.0f;
float fbImuActual_Smooth = 0.0f;
float fbServoDemand = 0.0f;
float fbServoDemand_Smooth = 0.0f;
float fbServoActual = 0.0f;
const float fbImu2servoScale = 4.0f;
RunningAverage fbImuSmoothing(50);
RunningAverage fbDemandSmoothing(50);

// Left to right axis
float lrControlValueBuff = 0.0f;
float lrControlValue = 0.0f;
float lrImuActual = 0.0f;
float lrImuActual_Smooth = 0.0f;
float lrServoDemand = 0.0f;
float lrServoDemand_Smooth = 0.0f;
float lrServoActual = 0.0f;
const float lrImu2servoScale = 4.0f;
RunningAverage lrImuSmoothing(50);
RunningAverage lrDemandSmoothing(50);

// Spin axis
float spinControlValueBuff = 0.0f;
float spinControlValue = 0.0f;
float spinVelocityDemand = 0.0f;


void dataHandler(uint8_t* packet, uint16_t size)
{
	if (size != (sizeof(int) + (3*sizeof(float)))) return;

	memcpy(&controlModeBuff, packet, sizeof(int));
	packet += sizeof(int);
	memcpy(&fbControlValueBuff, packet, sizeof(float));
	packet += sizeof(float);
	memcpy(&lrControlValueBuff, packet, sizeof(float));
	packet += sizeof(float);
	memcpy(&spinControlValueBuff, packet, sizeof(float));
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
	success &= fbServo.setMaxVelocity(90.0);

	LynxMotionServo lrServo(servoPort, 1);
	success &= lrServo.init();
	success &= lrServo.setMaxVelocity(90.0);

	LynxMotionServo spinServo(servoPort, 2);
	success &= spinServo.init();

	if (!success) abort("Failed to initialise servos");

	MPU6050 imu;
	imu.setPorts(IMU_I2C, IMU_SDA, IMU_SCL);
	imu.init(IMU_BASE_ID);
	imu.calcOffsets(false, false, true, true, true, true);   // Stabilises sensor
	imu.setInclinationOffsets(-3.6, -2.0, 0.0);				 // Accounts for error in mounting
	if (!imu.test()) abort("IMU_BASE not found");

	cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
	printf("Initialisation complete!\n");

	lrServo.setPositionOffset(7.0f);		// Positive = Right
	fbServo.setPositionOffset(-45.0f);		// Positive = Forward

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
		fbImuActual = -imu.inclination().x;

		lrImuSmoothing.append(lrImuActual);
		fbImuSmoothing.append(fbImuActual);

		controlMode = controlModeBuff;

		if (lrServo.update())
		{
		 	success = lrServo.getPosition(lrServoActual);
			if (success)
			{

				if (lrImuSmoothing.isFull())
				{
					lrImuActual_Smooth = lrImuSmoothing.getAverage();
					lrServoDemand = lrImuActual_Smooth * lrImu2servoScale;
					lrDemandSmoothing.append(lrServoDemand);
				}

				if (lrDemandSmoothing.isFull())
				{
					lrServoDemand_Smooth = lrDemandSmoothing.getAverage();

					if (controlMode == cm_auto)
					{
						success = lrServo.setPosition(lrServoDemand_Smooth);
					}
					else if (controlMode == cm_manual)
					{
						lrControlValue = lrControlValueBuff;
						success = lrServo.setPosition(lrControlValue * lrImu2servoScale);
					}
					else
					{
						success = lrServo.setPosition(0.0f);
					}
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
			success = fbServo.getPosition(fbServoActual);
			if (success)
			{

				if (fbImuSmoothing.isFull())
				{
					fbImuActual_Smooth = fbImuSmoothing.getAverage();
					fbServoDemand = fbImuActual_Smooth * fbImu2servoScale;
					fbDemandSmoothing.append(fbServoDemand);
				}

				if (fbDemandSmoothing.isFull())
				{
					fbServoDemand_Smooth = fbDemandSmoothing.getAverage();

					if (controlMode == cm_auto)
					{
						success = fbServo.setPosition(fbServoDemand_Smooth);
					}
					else if (controlMode == cm_manual)
					{
						fbControlValue = fbControlValueBuff;
						success = fbServo.setPosition(fbControlValue * fbImu2servoScale);
					}
					else
					{
						success = fbServo.setPosition(0.0f);
					}
				}
			}
			if (!success) msg += "Failed to set fbServo postition";
		}
		else 
		{
			msg += "Failed to update fbServo!";
		}

		if (spinServo.update())
		{
			if (controlMode == cm_manual)
			{
				spinControlValue = spinControlValueBuff;
				success = spinServo.setPosition(spinControlValue);
			}
		}

		if ((now - lastPrint) > 100)
		{
			//printf("%f %f %f %s\n", lrImuActual_Smooth, lrServoDemand_Smooth, lrServoActual, msg.c_str());
			//printf("%f %f %f %s\n", fbImuActual_Smooth, fbServoDemand_Smooth, fbServoActual, msg.c_str());
			printf("%d\n", controlMode);
			msg =  "";
			lastPrint = now;
		}

		if ((now - lastBlink) > 100)
		{
			ledState = !ledState;
			cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, ledState);
			lastBlink = now;
		}
	}

	finish();
	return 0;
}