#include "DomeMixer.h"
#include "MPU6050.h"

using namespace BB8;

DomeMixer::DomeMixer(MPU6050& _imuDome, MPU6050& _imuBase) :
	imuDome(_imuDome), imuBase(_imuBase)
{
	domeToBaseOffsets.x = 0;
	domeToBaseOffsets.y = 0;
	domeToBaseOffsets.z = 0;
}

void DomeMixer::update()
{
	imuDome.update();
	imuBase.update();

	float mixedZ = (imuDome.inclination().z + imuBase.inclination().z) / 2;

	domeFrame.x =  imuDome.inclination().x;
	domeFrame.y =  imuDome.inclination().y;
	domeFrame.z =  mixedZ;

	baseFrame.x =  imuBase.inclination().y;
	baseFrame.y = -imuBase.inclination().x;
	baseFrame.z =  mixedZ;

	domeToBaseFrame.x = (domeFrame.x - baseFrame.x) + domeToBaseOffsets.x;
	domeToBaseFrame.y = (domeFrame.y - baseFrame.y) + domeToBaseOffsets.y;
	domeToBaseFrame.z = (domeFrame.z - baseFrame.z) + domeToBaseOffsets.z;
}

void BB8::DomeMixer::setDomeToBaseOffsets(float x, float y, float z)
{
	domeToBaseOffsets.x = x;
	domeToBaseOffsets.y = y;
	domeToBaseOffsets.z = z;
}
