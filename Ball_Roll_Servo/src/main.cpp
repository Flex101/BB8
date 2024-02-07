#include "btController.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include <stdio.h>

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

	std::vector<ByteArray> recievedPackets;
	float axisValue = 0.0;

	while(true)
	{
		btController.poll();
		btController.getRecievedPackets(recievedPackets);

		for (ByteArray packet : recievedPackets)
		{
			if (packet.size() != 4)
			{
				printf("BAD PACKET!\n");
			}
			else
			{
				memcpy(&axisValue, &packet[0], 4);
			}

			printf("RCV: ");
			for (Byte byte : packet)
			{
				printf("%02x ", byte);
			}
			printf("%f", axisValue);
			printf("\n");
		}
		recievedPackets.clear();
	}
}
