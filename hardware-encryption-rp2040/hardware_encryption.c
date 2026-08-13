#include "pico/stdlib.h"
#include "pico/rand.h"
#include "ws2812.pio.h"
#include "tusb.h"
#include "sodium.h"

#include "crypto_helper.h"

#define LED_PIN 16
#define LED_INTERVAL_MS 10

const unsigned char DEVICE_PUBLIC_KEY[] = { 0x27, 0x56, 0x30, 0x1B, 0xF0, 0x3B, 0xFA, 0x28, 0xEB, 0xD1, 0x35, 0xFF, 0x48, 0xF2, 0xC5, 0x54, 0x6C, 0x98, 0x8C, 0x4C, 0x1D, 0x61, 0xA2, 0xB4, 0x07, 0x99, 0x4A, 0x40, 0xDB, 0x97, 0x31, 0x0B };
const unsigned char DEVICE_SECRET_KEY[] = { 0xF0, 0xB7, 0x07, 0x1B, 0xF7, 0xEF, 0x4E, 0x9D, 0xAB, 0x86, 0x42, 0x6B, 0x41, 0x3A, 0x18, 0xEF, 0xB2, 0x1E, 0x9E, 0xBF, 0x6F, 0x8D, 0xDC, 0x6A, 0x9A, 0x51, 0x48, 0x6A, 0x1E, 0x19, 0x9E, 0xDB };

PIO pio;
uint sm;
uint offset;

uint8_t g_rx_buff[64] = {0};
uint8_t g_rx_size = 0;
volatile bool g_data_received = false;

volatile bool g_is_encrypted_key_started = false;
unsigned char encrypted_key[80] = { 0 };
unsigned char program_public_key[crypto_box_PUBLICKEYBYTES] = { 0 };
unsigned char key_to_send[32] = { 0 };

static uint32_t led_last_ms = 0;

const char * pico_rng_name(void) {
	return "pico_rand_sdk";
}

void pico_random_buf(void *const buf, const size_t size) {
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

static inline void init_led() {
	bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio, &sm, &offset, LED_PIN, 1, true);
	hard_assert(success);

	ws2812_program_init(pio, sm, offset, LED_PIN, 800000, false);
}

static inline void handle_received_data() {
	if (!g_data_received || !tud_hid_ready()) {
		return;
	}

	char data[64] = {0};
	memcpy(data, g_rx_buff, g_rx_size);

	char flag = data[0];
	if (flag == 1) {
		if (!g_is_encrypted_key_started) {
			memcpy(encrypted_key, &data[1], 63);
			g_is_encrypted_key_started = true;
		}
		else {
			memcpy(&encrypted_key[63], &data[1], 80 - 63);
		}
	}
	else if (flag == 2) {
		memcpy(program_public_key, &data[1], crypto_box_PUBLICKEYBYTES);

		unsigned char decrypted_key[32] = { 0 };
		if (crypto_box_seal_open(decrypted_key, encrypted_key, 80, DEVICE_PUBLIC_KEY, DEVICE_SECRET_KEY) != 0) {
			set_led_color(rgb_color(255, 0, 0));
		}

		unsigned char response[80];
		crypto_box_seal(response, decrypted_key, 32, program_public_key);

		g_data_received = false;
		g_is_encrypted_key_started = false;

		unsigned char first_part[64] = { 0 };
		memcpy(first_part, response, 64);
		unsigned char second_part[64] = { 0 };
		memcpy(second_part, &response[64], 80 - 64);

		tud_hid_report(0, first_part, 64);

		while (!tud_hid_ready()) {
			tud_task();
		}

		tud_hid_report(0, second_part, 64);
	}
}

int main() {
	tusb_rhport_init_t dev_init = {
		.role = TUSB_ROLE_DEVICE,
		.speed = TUSB_SPEED_AUTO
	};

	init_led();

	tusb_init(0, &dev_init);

	static struct randombytes_implementation pico_rng;

	pico_rng.implementation_name = pico_rng_name;
	pico_rng.random = get_rand_32;
	pico_rng.stir = NULL;
	pico_rng.buf = pico_random_buf;

	randombytes_set_implementation(&pico_rng);

	if (sodium_init() < 0) {
		while (1) {
			set_led_color(rgb_color(255, 0, 0));
		}
	}

	while (1) {
		tud_task();

		handle_received_data();
	}
}

void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
	(void) itf; (void) report_id; (void) report_type;

	memcpy(g_rx_buff, buffer, bufsize > 64 ? 64 : bufsize);
	g_rx_size = bufsize;
	g_data_received = true;
}

uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
	(void) itf; (void) report_id; (void) report_type; (void) buffer; (void) reqlen;
	return 0;
}

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len) {
	(void) instance; (void) report; (void) len;
}
