#pragma once
#include <stdint.h>

#define BUFFER_SIZE 256

class DemandInput
{
public:
	explicit DemandInput();
	virtual ~DemandInput() {}

	void init();
	bool update();
	
	bool hasNewDemandPos();
	float getDemandPos();

private:
	char buffer[BUFFER_SIZE];
	uint8_t bufferLength;
	uint8_t bufferPos;

	bool hasNewDemand;
	float demandPos;
};