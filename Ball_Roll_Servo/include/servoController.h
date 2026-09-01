#pragma once
#include <string>

class ServoController
{
public:
	explicit ServoController();
	virtual ~ServoController() {}

	bool init();

	bool setMaxSpeed(int value);
	bool setSpeedDamping(int value);
	bool setPGain(int value);

	bool readPos(int& result);
	bool writePos(int demand);

	bool writeVel(int demand);

protected:
	void clearBuffer(std::string& result);
	bool readLine(std::string& result);
	void nicePrint(const std::string& str);
	void nicePrint(const bool& str);
};