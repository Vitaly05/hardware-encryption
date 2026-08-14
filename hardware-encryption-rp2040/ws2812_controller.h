#ifndef _WS2812_CONTROLLER_H_
#define _WS2812_CONTROLLER_H_

#include "pico/stdlib.h"
#include "ws2812.pio.h"

#define LED_PIN 16
#define LED_INTERVAL_MS 10

static uint32_t led_last_ms = 0;
static PIO pio;
static uint sm;
static uint offset;

static inline void init_led() {
	bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio, &sm, &offset, LED_PIN, 1, true);
	hard_assert(success);

	ws2812_program_init(pio, sm, offset, LED_PIN, 800000, false);
}

static inline void set_led_color(uint32_t pixel_grb) {
	uint32_t current_ms = to_ms_since_boot(get_absolute_time());

	if (current_ms - led_last_ms >= LED_INTERVAL_MS) {
		led_last_ms = current_ms;

		pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
	}
}

static inline uint32_t rgb_color(uint8_t r, uint8_t g, uint8_t b) {
	return ((uint32_t)(g) << 16) | ((uint32_t)(r) << 8) | (uint32_t)(b);
}

#endif // _WS2812_CONTROLLER_H_
