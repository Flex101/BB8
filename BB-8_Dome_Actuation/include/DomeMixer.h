#pragma once
#include "Common.h"

class MPU6050;

namespace BB8
{

class DomeMixer
{
public:
	explicit DomeMixer(MPU6050& imuDome, MPU6050& imuBase);
	virtual ~DomeMixer() {}

	void init();
	void update();

	const Frame& dome()       { return domeFrame; }
	const Frame& domeToBase() { return domeToBaseFrame; }
	const Frame& base()       { return baseFrame; }

private:
	MPU6050& imuDome;
	MPU6050& imuBase;

	Frame domeFrame;
	Frame domeToBaseFrame;
	Frame baseFrame;
};

} // namespace BB8