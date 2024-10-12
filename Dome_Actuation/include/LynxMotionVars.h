#pragma once
#include <string>


const std::string ID = "ID";
const std::string ANGULAR_RANGE = "AR";
const std::string POSITION_DEG = "D";
const std::string VELOCITY_DEGSEC = "WD";
const std::string LIMP = "L";
const std::string HALT_HOLD = "H";

const float MAX_VELOCITY = 180.0f;

namespace RangeStatus {

	const float NEAR_RANGE = 5.0f;

	enum Enum
	{
		UNKNOWN = 0,
		PAST_NEG_LIMIT = 1,
		NEAR_NEG_LIMIT = 2,
		WITHIN_RANGE = 3,
		NEAR_POS_LIMIT = 4,
		PAST_POS_LIMIT = 5
	};
}