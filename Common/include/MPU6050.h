#pragma once
#include "Common.h"

struct RawFrame
{
	int16_t x;
	int16_t y;
	int16_t z;
};

class MPU6050
{
public:
	explicit MPU6050();
	virtual ~MPU6050() {}

	byte init(byte dev_addr = 0x68, bool init_i2c = true, int gyro_config = 1, int accel_config = 0);
	void calcOffsets(bool accel_x = true, bool accel_y = true, bool accel_z = true, bool gyro_x = true, bool gyro_y = true, bool gyro_z = true);

	bool test();
	byte setGyroConfig(int config_num);
	byte setAccelConfig(int config_num);
	void setGyroOffsets(float x, float y, float z);
	void setAccelOffsets(float x, float y, float z);
	void setFilterGyroCoef(float gyro_coef);
	void setFilterAccelCoef(float accel_coef);
	void update();

	const Frame& gyro()        { return gyroFrame; }
	const Frame& accel()       { return accelFrame; }
	const Frame& inclination() { return inclinationFrame; }

private:
	void fetchData();
	void resetImu();

private:
	const byte IMU_ID           = 0x68; // Doesn't change with address
	const byte IMU_SDA          = 16;
	const byte IMU_SCL          = 17;
	const byte REG_DEV_ID       = 0x75;
	const byte REG_CONFIG       = 0x1A;
	const byte REG_GYRO_CONFIG  = 0x1B;
	const byte REG_ACCEL_CONFIG = 0x1C;
	const byte REG_DATA         = 0x3B;
	const byte REG_DATA_LEN     = 14;
	const byte REG_PWR_MGMT_1   = 0x6B;

	const float DEFAULT_GYRO_COEFF = 0.98;
	const float RAD_2_DEG          = 57.29578; // [deg/rad]
	const int CALIB_OFFSET_NB_MES  = 500;

	float gyroToDegSec;
	float accelToG;
	float filterGyroCoef; // complementary filter coefficient to balance gyro vs accelero data to get angle

	byte devAddr;
	uint32_t preInterval;
	byte buffer[14];
	RawFrame rawGyroFrame;
	RawFrame rawAccelFrame;
	Frame gyroOffsets;
	Frame accelOffsets;
	Frame gyroFrame;
	Frame accelFrame;
	Frame angleAccelFrame;
	Frame inclinationFrame;
};