#include "LynxMotionPort.h"
#include "Common.h"
#include "pico/stdlib.h"
#include <stdio.h>

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

bool LynxMotionPort::send(const std::string& outgoingMsg, std::string& incomingMsg)
{
	while(!uart_is_writable(uartPort))
	{
		sleep_ms(1);
	}

	uart_puts(uartPort, outgoingMsg.c_str());
	nicePrint("Sent: " + outgoingMsg);
	bool success = readLine(incomingMsg);
	nicePrint("Reply: " + incomingMsg);
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