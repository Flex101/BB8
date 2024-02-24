#pragma once
#include <string>

class ServoController
{
public:
	explicit ServoController();
	virtual ~ServoController() {}

	bool init();

	bool setMaxSpeed(int speed);

	bool readPos(int& result);
	bool writePos(int demand);

	bool writeVel(int demand);

protected:
	bool readLine(std::string& result);
	void nicePrint(const std::string& str);
	void nicePrint(const bool& str);
};