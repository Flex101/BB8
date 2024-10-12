#include "LynxMotionPort.h"
#include "Common.h"
#include "pico/stdlib.h"
#include <stdio.h>

const bool DEBUG_MSG = false;
const uint BAUD_RATE = 115200;
const uint DATA_BITS = 8;
const uint STOP_BITS = 1;
const uart_parity_t PARITY = UART_PARITY_NONE;

LynxMotionPort::LynxMotionPort(uart_inst_t* uartPort, uint8_t uartTxPin, uint8_t uartRxPin) :
	uartPort(uartPort), uartTxPin(uartTxPin), uartRxPin(uartRxPin)
{
}

bool LynxMotionPort::init()
{
	uart_init(uartPort, BAUD_RATE);
	gpio_set_function(uartTxPin, GPIO_FUNC_UART);
	gpio_set_function(uartRxPin, GPIO_FUNC_UART);
	uart_set_hw_flow(uartPort, false, false);
	uart_set_format(uartPort, DATA_BITS, STOP_BITS, PARITY);
	uart_set_fifo_enabled(uartPort, false);

	return true;
}

void LynxMotionPort::send(const std::string &outgoingMsg)
{
	while(!uart_is_writable(uartPort))
	{
		if (DEBUG_MSG) nicePrint("Wating to send...");
		sleep_ms(1);
	}

	if (DEBUG_MSG) nicePrint("Sending...");
	uart_puts(uartPort, outgoingMsg.c_str());
	if (DEBUG_MSG) nicePrint("Sent: " + outgoingMsg);
}

bool LynxMotionPort::send(const std::string& outgoingMsg, std::string& incomingMsg)
{
	send(outgoingMsg);

	bool success = readLine(incomingMsg);
	if (DEBUG_MSG)
	{
		if (success) nicePrint("Reply: " + incomingMsg);
		else nicePrint("Reply: FAILED!");
	}
	return success;
}

bool LynxMotionPort::readLine(std::string& result)
{
	std::string reply = "";

	uint32_t start = to_ms_since_boot(get_absolute_time());
	while(true)
	{
		if (!uart_is_readable_within_us(uartPort, 10000)) // 10ms
		{
			result = reply;
			return false;
		}

		char ch = uart_getc(uartPort);
		reply += char(ch);

		if (ch == '\r') break;
	}

	result = reply;
	return true;
}
