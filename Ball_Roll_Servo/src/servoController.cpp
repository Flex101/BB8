#include "servoController.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include <stdio.h>

#define UART_ID uart0
#define BAUD_RATE 9600
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE
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
	if (!success) return false;

	return (reply == expected);
}

bool ServoController::readPos(int& result)
{
	uart_puts(UART_ID, "P\r\n");

	std::string reply;
	bool success = readLine(reply);
	if (!success) return false;
	if (reply[0] != ':') return false;
	if (reply[1] != 'P') return false;

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

bool ServoController::readLine(std::string& result)
{
	std::string reply;

	uint32_t start = to_ms_since_boot(get_absolute_time());
	while(true)
	{
		char ch = uart_getc(UART_ID);
		printf("%c", ch);
		reply += char(ch);
		if (ch == '\n') break;
		if (!uart_is_readable_within_us(UART_ID, 2000)) return false;
	}
	printf("\n");

	result = reply;
	return true;
}
