#pragma once
#include <string>

class ServoController
{
public:
	explicit ServoController();
	virtual ~ServoController() {}

	bool init();

	bool readPos(int& result);
	bool writePos(int demand);

protected:
	bool readLine(std::string& result);
};