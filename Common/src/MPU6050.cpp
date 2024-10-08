#include "MPU6050.h"
#include "Common.h"
#include "math.h"
#include <stdio.h>

/* Wrap an angle in the range [-limit,+limit] (special thanks to Edgar Bonet!) */
static float wrap(float angle, float limit)
{
	while (angle >  limit) angle -= 2*limit;
	while (angle < -limit) angle += 2*limit;
	return angle;
}

MPU6050::MPU6050()
{
	inclinationFrame.x = 0.0;
	inclinationFrame.y = 0.0;
	inclinationFrame.z = 0.0;

	offsetInclinationFrame.x = 0.0;
	offsetInclinationFrame.y = 0.0;
	offsetInclinationFrame.z = 0.0;

	setFilterGyroCoef(DEFAULT_GYRO_COEFF);
	setGyroOffsets(0,0,0);
	setAccelOffsets(0,0,0);
	setInclinationOffsets(0,0,0);
}

void MPU6050::setPorts(byte i2c, byte sda, byte scl)
{
	if (i2c == 1) IMU_INST = i2c1;
	else IMU_INST = i2c0;

	IMU_SDA = sda;
	IMU_SCL = scl;
}

byte MPU6050::init(byte dev_addr, bool init_i2c, int gyro_config, int accel_config)
{
	if (init_i2c)
	{
		i2c_init(IMU_INST, 400 * 1000);
		gpio_set_function(IMU_SDA, GPIO_FUNC_I2C);
		gpio_set_function(IMU_SCL, GPIO_FUNC_I2C);
		gpio_pull_up(IMU_SDA);
		gpio_pull_up(IMU_SCL);
	}

	devAddr = dev_addr;
	resetImu();
	setGyroConfig(gyro_config);
	setAccelConfig(accel_config);

	preInterval = millis(); // Must call this immediately before first update
	update();
	inclinationFrame.x = angleAccelFrame.x;
	inclinationFrame.y = angleAccelFrame.y;
	
	return false;
}

void MPU6050::calcOffsets(bool accel_x, bool accel_y, bool accel_z, bool gyro_x, bool gyro_y, bool gyro_z)
{
	if (accel_x) { accelOffsets.x = 0; }
	if (accel_y) { accelOffsets.y = 0; }
	if (accel_z) { accelOffsets.z = 0; }
	if (gyro_x)  { gyroOffsets.x  = 0; }
	if (gyro_y)  { gyroOffsets.x  = 0; }
	if (gyro_z)  { gyroOffsets.x  = 0; }

	float ag[6] = {0,0,0,0,0,0}; // 3*acc, 3*gyro

	for(int i = 0; i < CALIB_OFFSET_NB_MES; i++)
	{
		this->fetchData();
		ag[0] += accelFrame.x;
		ag[1] += accelFrame.y;
		ag[2] += (accelFrame.z-1.0);
		ag[3] += gyroFrame.x;
		ag[4] += gyroFrame.y;
		ag[5] += gyroFrame.z;
		sleep_ms(1); // wait a little bit between 2 measurements
	}

	if (accel_x) { accelOffsets.x = ag[0] / CALIB_OFFSET_NB_MES; }
	if (accel_y) { accelOffsets.y = ag[1] / CALIB_OFFSET_NB_MES; }
	if (accel_z) { accelOffsets.z = ag[2] / CALIB_OFFSET_NB_MES; }
	if (gyro_x)  { gyroOffsets.x  = ag[3] / CALIB_OFFSET_NB_MES; }
	if (gyro_y)  { gyroOffsets.y  = ag[4] / CALIB_OFFSET_NB_MES; }
	if (gyro_z)  { gyroOffsets.z  = ag[5] / CALIB_OFFSET_NB_MES; }
}

bool MPU6050::test()
{
	byte buf;
	i2c_write_blocking(IMU_INST, devAddr, &REG_DEV_ID, 1, true); 
	i2c_read_blocking(IMU_INST, devAddr, &buf, 1, false);
	return (buf == IMU_ID);
}

byte MPU6050::setGyroConfig(int config_num)
{
	byte msg[] = {REG_GYRO_CONFIG, 0x00};

	switch (config_num)
	{
		case 0: // range = +-250 deg/s
			gyroToDegSec = 131.0;
			msg[1] = 0x00;
			break;

		case 1: // range = +-500 deg/s
			gyroToDegSec = 65.5;
			msg[1] = 0x08;
			break;
		
		case 2: // range = +-1000 deg/s
			gyroToDegSec = 32.8;
			msg[1] = 0x10;
			break;

		case 3: // range = +-2000 deg/s
			gyroToDegSec = 16.4;
			msg[1] = 0x18;
			break;

		default:
			return 1;
	}

	return i2c_write_blocking(IMU_INST, devAddr, msg, 2, true); 
}

byte MPU6050::setAccelConfig(int config_num)
{
	byte msg[] = {REG_ACCEL_CONFIG, 0x00};

	switch (config_num)
	{
		case 0: // range = +-2 g
			accelToG = 16384.0;
			msg[1] = 0x00;
			break;

		case 1: // range = +-4 g
			accelToG = 8192.0;
			msg[1] = 0x08;
			break;
		
		case 2: // range = +-8 g
			accelToG = 4096.0;
			msg[1] = 0x10;
			break;

		case 3: // range = +-16 g
			accelToG = 2048.0;
			msg[1] = 0x18;
			break;

		default:
			return 1;
	}

	return i2c_write_blocking(IMU_INST, devAddr, msg, 2, true);
}

void MPU6050::setGyroOffsets(float x, float y, float z)
{
	gyroOffsets.x = x;
	gyroOffsets.y = y;
	gyroOffsets.z = z;
}

void MPU6050::setAccelOffsets(float x, float y, float z)
{
	accelOffsets.x = x;
	accelOffsets.y = y;
	accelOffsets.z = z;
}

void MPU6050::setInclinationOffsets(float x, float y, float z)
{
	inclinationOffsets.x = x;
	inclinationOffsets.y = y;
	inclinationOffsets.z = z;
}

void MPU6050::setFilterGyroCoef(float gyro_coef)
{
	if ((gyro_coef < 0) or (gyro_coef > 1))
		gyro_coef = DEFAULT_GYRO_COEFF; // prevent bad gyro coeff, should throw an error...
	filterGyroCoef = gyro_coef;
}

void MPU6050::setFilterAccelCoef(float acc_coeff)
{
	setFilterGyroCoef(1.0-acc_coeff);
}

void MPU6050::update()
{
	this->fetchData();

	// estimate tilt angles: this is an approximation for small angles!
	float sgZ = accelFrame.z < 0 ? -1 : 1; // allow one angle to go from -180 to +180 degrees
	angleAccelFrame.x =   atan2(accelFrame.y, sgZ*sqrt(accelFrame.z*accelFrame.z + accelFrame.x*accelFrame.x)) * RAD_2_DEG; // [-180,+180] deg
	angleAccelFrame.y = - atan2(accelFrame.x,     sqrt(accelFrame.z*accelFrame.z + accelFrame.y*accelFrame.y)) * RAD_2_DEG; // [- 90,+ 90] deg

	uint32_t Tnew = millis();
	float dt = (Tnew - preInterval) * 1e-3;
	preInterval = Tnew;

	inclinationFrame.x  = wrap(filterGyroCoef*(angleAccelFrame.x + wrap(inclinationFrame.x +     gyroFrame.x*dt - angleAccelFrame.x,180)) + (1.0-filterGyroCoef)*angleAccelFrame.x,180);
	inclinationFrame.y  = wrap(filterGyroCoef*(angleAccelFrame.y + wrap(inclinationFrame.y + sgZ*gyroFrame.y*dt - angleAccelFrame.y, 90)) + (1.0-filterGyroCoef)*angleAccelFrame.y, 90);
	inclinationFrame.z += gyroFrame.z * dt; // not wrapped

	offsetInclinationFrame.x = inclinationFrame.x - inclinationOffsets.x;
	offsetInclinationFrame.y = inclinationFrame.y - inclinationOffsets.y;
	offsetInclinationFrame.z = inclinationFrame.z - inclinationOffsets.z;
}

void MPU6050::fetchData()
{
	i2c_write_blocking(IMU_INST, devAddr, &REG_DATA, 1, true);
	i2c_read_blocking(IMU_INST, devAddr, buffer, REG_DATA_LEN, false);

	// for (int i = 0; i < REG_DATA_LEN; ++i)
	// {
	// 	printf("%02x ", buffer[i]);
	// }
	// printf("\n");

	rawAccelFrame.x = buffer[ 0]<<8 | buffer[ 1];
	rawAccelFrame.y = buffer[ 2]<<8 | buffer[ 3];
	rawAccelFrame.z = buffer[ 4]<<8 | buffer[ 5];
	rawGyroFrame.x  = buffer[ 8]<<8 | buffer[ 9];
	rawGyroFrame.y  = buffer[10]<<8 | buffer[11];
	rawGyroFrame.z  = buffer[12]<<8 | buffer[13];

	// printf("%d %d %d %d %d %d\n", rawAccelFrame.x, rawAccelFrame.y, rawAccelFrame.z, rawGyroFrame.x, rawGyroFrame.y, rawGyroFrame.z);

	accelFrame.x = (((float)rawAccelFrame.x) / accelToG) - accelOffsets.x;
	accelFrame.y = (((float)rawAccelFrame.y) / accelToG) - accelOffsets.y;
	accelFrame.z = (((float)rawAccelFrame.z) / accelToG) - accelOffsets.z;
	gyroFrame.x = (((float)rawGyroFrame.x) / gyroToDegSec) - gyroOffsets.x;
	gyroFrame.y = (((float)rawGyroFrame.y) / gyroToDegSec) - gyroOffsets.y;
	gyroFrame.z = (((float)rawGyroFrame.z) / gyroToDegSec) - gyroOffsets.z;

	// printf("%f %f %f %f %f %f\n", accelFrame.x, accelFrame.y, accelFrame.z, gyroFrame.x, gyroFrame.y, gyroFrame.z);
}

void MPU6050::resetImu()
{
	byte msg[] = {REG_PWR_MGMT_1, 0x00};
 	i2c_write_blocking(IMU_INST, devAddr, msg, 2, false);
}
