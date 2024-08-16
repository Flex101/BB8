#pragma once

class TiltController
{
public:
	explicit TiltController();
	virtual ~TiltController() {}

	bool init();
	bool writePos(float demand);
};