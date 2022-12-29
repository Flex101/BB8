#include "pico/stdlib.h"

using byte = uint8_t;

struct Frame
{
	int16_t x;
	int16_t y;
	int16_t z;
};

class MPU6050
{
public:

public:
	explicit MPU6050() {}
	virtual ~MPU6050() {}

	byte init(int gyro_config = 1, int accel_config = 0);

	bool test();
	byte setGyroConfig(int config_num);
	byte setAccConfig(int config_num);
	void update();

	Frame& gyro()  { return gyroFrame; }
	Frame& accel() { return accelFrame; }

private:
	void reset();
	int16_t toInt(byte val_h, byte val_l);

private:
	const byte DEV_ADDR     = 0x68;
	const byte DEV_ID       = 0x68;
	const byte IMU_SDA      = 16;
	const byte IMU_SCL      = 17;
	const byte REG_DEV_ID   = 0x75;
	const byte REG_DATA     = 0x3B;
	const byte REG_DATA_LEN = 14;

	byte buffer[14];
	Frame gyroFrame;
	Frame accelFrame;
};