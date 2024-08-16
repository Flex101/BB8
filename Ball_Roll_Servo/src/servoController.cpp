#include "servoController.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include <stdio.h>

#define UART_ID uart0
#define BAUD_RATE 9600
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE
#define UART_TX_PIN 16
#define UART_RX_PIN 17

ServoController::ServoController()
{
}

bool ServoController::init()
{
	uart_init(UART_ID, BAUD_RATE);
	gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
	uart_set_hw_flow(UART_ID, false, false);
	uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);
	uart_set_fifo_enabled(UART_ID, false);


	// Reset encoder position
	uart_puts(UART_ID, "P0\r\n");
	std::string expected = "P0\r\n";
	std::string reply;
	bool success = readLine(reply);
	nicePrint(reply);
	if (!success) return false;
	if (reply != expected) return false;


	// Reset gains incase they got corrupted
	uart_puts(UART_ID, "Y\r\n");
	expected = "Y";
	success = readLine(reply);
	nicePrint(reply);
	if (reply != expected) return false;

	return true;
}

bool ServoController::setMaxSpeed(int speed)
{
	if (speed > 255) return false;
	if (speed < 0) return false;

	std::string msg = "M" + std::to_string(speed) + "\r\n";
	uart_puts(UART_ID, msg.c_str());

	std::string expected = msg;
	std::string reply;
	bool success = readLine(reply);
	nicePrint(success);
	nicePrint(reply);
	if (!success) return false;
	

	return (reply == expected);
}

bool ServoController::readPos(int& result)
{
	uart_puts(UART_ID, "P\r\n");

	std::string reply;
	bool success = readLine(reply);
	if (!success) return false;
	if (reply[0] != 'P') return false;

	reply = reply.substr(2, reply.length() - 5);
	result = std::atoi(reply.c_str());

	return true;
}

bool ServoController::writePos(int demand)
{
	std::string msg = "G";
	msg += std::to_string(demand);
	msg += "\r\n";

	uart_puts(UART_ID, msg.c_str());

	std::string reply;
	bool success = readLine(reply);
	//printf("WRITE REPLY: %s\n", reply.c_str());

	return success;
}

bool ServoController::writeVel(int demand)
{
	if (demand > 99) return false;
	if (demand < -99) return false;

	std::string msg = "S";
	if (demand >0 ) msg += "+";
	msg += std::to_string(demand);
	msg += "\r\n";

	uart_puts(UART_ID, msg.c_str());

	std::string reply;
	bool success = readLine(reply);
	//printf("WRITE REPLY: %s\n", reply.c_str());

	return success;
}

bool ServoController::readLine(std::string& result)
{
	std::string reply;

	uint32_t start = to_ms_since_boot(get_absolute_time());
	while(true)
	{
		char ch = uart_getc(UART_ID);

		if (ch == ':') break;
		reply += char(ch);

		if (!uart_is_readable_within_us(UART_ID, 2000))
		{
			result = reply;
			return false;
		}
	}

	result = reply;
	return true;
}

void ServoController::nicePrint(const std::string& str)
{
	for (char ch : str)
	{
		if (ch == '\r') printf("\\r");
		else if (ch == '\n') printf("\\n");
		else printf("%c", ch);
	}
	printf("\n");
}

void ServoController::nicePrint(const bool& val)
{
	if (val) printf("True\n");
	else printf("False\n");
}