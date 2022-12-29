#include "MPU6050.h"
#include "hardware/i2c.h"

byte MPU6050::init(int gyro_config, int accel_config)
{
	i2c_init(i2c_default, 400 * 1000);
	gpio_set_function(IMU_SDA, GPIO_FUNC_I2C);
 	gpio_set_function(IMU_SCL, GPIO_FUNC_I2C);
 	gpio_pull_up(IMU_SDA);
 	gpio_pull_up(IMU_SCL);
	reset();
	
	return false;
}

bool MPU6050::test()
{
	byte buf;
	i2c_write_blocking(i2c_default, DEV_ADDR, &REG_DEV_ID, 1, true); 
	i2c_read_blocking(i2c_default, DEV_ADDR, &buf, 1, false);
	return (buf == DEV_ID);
}

byte MPU6050::setGyroConfig(int config_num)
{
	return 0;
}

byte MPU6050::setAccConfig(int config_num)
{
	return 0;
}

void MPU6050::update()
{
	i2c_write_blocking(i2c_default, DEV_ADDR, &REG_DATA, 1, true);
	i2c_read_blocking(i2c_default, DEV_ADDR, buffer, REG_DATA_LEN, false);

	accelFrame.x = buffer[ 0]<<8 | buffer[ 1];
	accelFrame.y = buffer[ 2]<<8 | buffer[ 3];
	accelFrame.z = buffer[ 4]<<8 | buffer[ 5];
	gyroFrame.x  = buffer[ 8]<<8 | buffer[ 9];
	gyroFrame.y  = buffer[10]<<8 | buffer[11];
	gyroFrame.z  = buffer[12]<<8 | buffer[13];
}

void MPU6050::reset()
{
	byte buf[] = {0x6B, 0x00};
 	i2c_write_blocking(i2c_default, DEV_ADDR, buf, 2, false);
}
