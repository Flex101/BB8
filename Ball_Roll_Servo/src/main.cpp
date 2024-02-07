#include "btController.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include <stdio.h>

float axisValue;

void dataHandler(uint8_t* packet, uint16_t size)
{
	if (size != 4) return;
	memcpy(&axisValue, packet, 4);
}

int main()
{
	stdio_init_all();

	// initialize CYW43 driver
	if (cyw43_arch_init())
	{
		printf("cyw43_arch_init() failed.\n");
		return -1;
	}

	cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

	BtController btController("BB-8 Ball Roll", 0xBB8);
	btController.setDataHandler(dataHandler);

	while(true)
	{
		btController.poll();
		printf("%f\n", axisValue);
	}
}
