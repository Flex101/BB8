#pragma once

#include "hardware/uart.h"
#include <string>
#include <stdio.h>

class LynxMotionPort
{
public:
	explicit LynxMotionPort(uart_inst_t* uartPort, uint8_t uartTxPin, uint8_t uartRxPin);
	virtual ~LynxMotionPort() {}

	bool init();

	bool send(const std::string& outgoingMsg, std::string& incomingMsg);

protected:
	bool readLine(std::string& result);

private:
	uart_inst_t* uartPort;
	uint8_t uartTxPin;
	uint8_t uartRxPin;

};