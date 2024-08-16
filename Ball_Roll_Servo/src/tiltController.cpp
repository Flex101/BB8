#include "tiltController.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include <stdio.h>
#include <cstring>

#define UART_ID uart1
#define BAUD_RATE 9600
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE
#define UART_TX_PIN 8
#define UART_RX_PIN 9

TiltController::TiltController()
{
}

bool TiltController::init()
{
	uart_init(UART_ID, BAUD_RATE);
	gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
	uart_set_hw_flow(UART_ID, false, false);
	uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);
	uart_set_fifo_enabled(UART_ID, false);

	return true;
}

bool TiltController::writePos(float demand)
{
	char msg[6];
	msg[0] = 'P';
	memcpy(&msg[1], &demand, 4);
	msg[5] = '\n';

	uart_puts(UART_ID, msg);
}
