#pragma once
#include "pico/stdlib.h"

using byte = uint8_t;

static uint32_t millis()
{
	return to_ms_since_boot(get_absolute_time());
}

struct Frame
{
	float x;
	float y;
	float z;
};