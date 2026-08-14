#include "sodium_config.h"

#include "pico/stdlib.h"
#include "pico/rand.h"
#include "sodium.h"

static const char * pico_rng_name(void) {
	return "pico_rand_sdk";
}

static void pico_random_buf(void *const buf, const size_t size) {
	uint8_t *p = (uint8_t *)buf;
	size_t i = 0;

	while (i + 4 <= size) {
		uint32_t r = get_rand_32();
		p[i]     = (uint8_t)(r & 0xFF);
		p[i + 1] = (uint8_t)((r >> 8) & 0xFF);
		p[i + 2] = (uint8_t)((r >> 16) & 0xFF);
		p[i + 3] = (uint8_t)((r >> 24) & 0xFF);
		i += 4;
	}

	if (i < size) {
		uint32_t r = get_rand_32();
		while (i < size) {
			p[i] = (uint8_t)(r & 0xFF);
			r >>= 8;
			i++;
		}
	}
}

void set_randombytes_implementation() {
	static struct randombytes_implementation pico_rng;

	pico_rng.implementation_name = pico_rng_name;
	pico_rng.random = get_rand_32;
	pico_rng.stir = NULL;
	pico_rng.buf = pico_random_buf;

	randombytes_set_implementation(&pico_rng);
}
