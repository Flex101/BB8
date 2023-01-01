#pragma once

using byte = uint8_t;

static uint32_t millis()
{
	return to_ms_since_boot(get_absolute_time());
}