#pragma once

namespace BB8
{

	class Mapping
	{
	public:
		explicit Mapping(float scalar, float offset);
		virtual ~Mapping() {}

		float angleToServo(const float& angle) const;
		float servoToAngle(const float& servo) const;

	private:
		// From servo (-1 to 1) to IMU (degrees)
		float scalar;
		float offset;
	};

} // namespace BB8