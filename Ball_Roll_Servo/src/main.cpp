#include "src/btstack/spp_streamer.c"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "btstack_run_loop.h"
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

	// run the app
    btstack_main(0, NULL);
    btstack_run_loop_execute();
}
