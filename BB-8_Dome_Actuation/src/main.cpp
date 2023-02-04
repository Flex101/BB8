#include "Control.h"
#include "Calibration.h"
#include "DomeMixer.h"
#include "Servo.h"
#include "MPU6050.h"
#include "Common.h"
#include <string>
#include <math.h>
#include <stdio.h>

const byte IMU_DOME_ID = 0x69;
const byte IMU_BASE_ID = 0x68;
const uint LED_PIN = 25;
const uint SERVO_FB_PIN = 27;
const uint SERVO_LR_PIN = 21;

void abort(const char *msg)
{
	while (true)
	{
		gpio_put(LED_PIN, 0);
		sleep_ms(500);
		gpio_put(LED_PIN, 1);
		sleep_ms(500);
		printf("%s\n", msg);
	}
}

int main()
{
	stdio_init_all();
	//sleep_ms(3000);

	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);
	gpio_put(LED_PIN, 1);

	Servo servoLr(SERVO_LR_PIN);
	servoLr.setPosRange(1150,1600);
	servoLr.setPosHome(1375);

	Servo servoFb(SERVO_FB_PIN);
	servoFb.setPosRange(1300,1650);
	servoFb.setPosHome(1475);

	servoFb.goToHome();
	servoLr.goToHome();
	sleep_ms(2000);

	MPU6050 imuDome;
	imuDome.init(IMU_DOME_ID);
	if (!imuDome.test()) abort("IMU_DOME not found");

	MPU6050 imuBase;
	imuBase.init(IMU_BASE_ID, false);
	if (!imuBase.test()) abort("IMU_BASE not found");

	// Tries to reduce z inclination drift while leaving x and y inclination relative to gravity
	imuDome.calcOffsets(false, false, true, true, true, true);
	imuBase.calcOffsets(false, false, true, true, true, true);

	BB8::DomeMixer domeMixer(imuDome, imuBase);
	domeMixer.setDomeToBaseOffsets(-2.5, 2.5, 0); // Fudge offset in mounting

	/*
	BB8::Calibration calibration(servoFb, servoLr, domeMixer);
	calibration.mapAxis(BB8::Direction::FB);
	calibration.mapAxis(BB8::Direction::LR);
	*/

	BB8::Control control(servoFb, servoLr, domeMixer);

	const float demands[] = {0, 10, -10, 20, -20, 10, -10 , 0,  0};
	const uint demandLen = 9;

	uint32_t outputTimer = millis();
	uint32_t demandTimer = millis();
	uint index = 0;
	bool hold = false;
	while(index < (demandLen - 1))
	{
		control.spin();
		
		if ((millis() - demandTimer) > 5000)
		{
			index = (index + 1) % demandLen;
			control.setDemand(BB8::Direction::LR, demands[index]);
			demandTimer = millis();
			hold = false;
		}

		if ((millis() - outputTimer) > 10)
		{
			printf("%f %f %f \n",
				demands[index],
				control.getCalcDemand(BB8::Direction::LR),
				domeMixer.domeToBase().x
			);
			outputTimer = millis();
		}
	}

	/*
	uint32_t timer = 0;
	while (true)
	{
		domeMixer.update();

		if ((millis() - timer) > 10)
		{
			printf("%f %f %f %f %f %f %f %f %f\n",
				   domeMixer.dome().x, domeMixer.dome().y, domeMixer.dome().z,
				   domeMixer.base().x, domeMixer.base().y, domeMixer.base().z,
				   domeMixer.domeToBase().x, domeMixer.domeToBase().y, domeMixer.domeToBase().z);
			timer = millis();
		}
	}
	*/

	finish();
	return 0;
}
