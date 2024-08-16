#include "DemandInput.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include <stdio.h>
#include <cstring>

#define UART_ID uart1
#define BAUD_RATE 9600
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE
#define UART_TX_PIN 8
#define UART_RX_PIN 9

DemandInput::DemandInput() :
	bufferLength(0),
	bufferPos(0),
	hasNewDemand(false),
	demandPos(0.0f)
{
}

void DemandInput::init()
{
	uart_init(UART_ID, BAUD_RATE);
	gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
	uart_set_hw_flow(UART_ID, false, false);
	uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);
	uart_set_fifo_enabled(UART_ID, false);
}

bool DemandInput::update()
{
	//printf("Update...\n");

	while (uart_is_readable(UART_ID))
	{
		char ch = uart_getc(UART_ID);
		buffer[bufferLength] = ch;
		bufferLength++;
		printf("%d\n", ch);
	}

	printf("%d %d\n", bufferLength, bufferPos);


	while ((bufferLength - bufferPos) >= 6)
	{
		if (buffer[bufferPos] != 'P')
		{
			bufferPos++;
			continue;
		}

		if (buffer[bufferPos + 5] != '\n')
		{
			// Corrupt! Clear buffer and start again
			bufferLength = 0;
			bufferPos = 0;
			break;
		}

		// Success! Store value and clear buffer
		memcpy(&demandPos, &buffer[bufferPos + 1], 4);
		hasNewDemand = true;
		bufferLength = 0;
		bufferPos = 0;
	}

	// for (int i = 0; i < bufferLength; ++i)
	// {
	// 	printf( "%d ", buffer[bufferPos + i]);
	// }
	// printf("\n");
}

bool DemandInput::hasNewDemandPos()
{
	return hasNewDemand;
}

float DemandInput::getDemandPos()
{
	hasNewDemand = false;
	return demandPos;
}
