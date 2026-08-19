#include "pico/stdlib.h"
#include "pico/rand.h"
#include "tusb.h"
#include "sodium.h"
#include "sodium_config.h"
#include "ws2812_controller.h"
#include "../shared/shared_config.h"

const unsigned char DEVICE_PUBLIC_KEY[] = { 0x27, 0x56, 0x30, 0x1B, 0xF0, 0x3B, 0xFA, 0x28, 0xEB, 0xD1, 0x35, 0xFF, 0x48, 0xF2, 0xC5, 0x54, 0x6C, 0x98, 0x8C, 0x4C, 0x1D, 0x61, 0xA2, 0xB4, 0x07, 0x99, 0x4A, 0x40, 0xDB, 0x97, 0x31, 0x0B };
const unsigned char DEVICE_SECRET_KEY[] = { 0xF0, 0xB7, 0x07, 0x1B, 0xF7, 0xEF, 0x4E, 0x9D, 0xAB, 0x86, 0x42, 0x6B, 0x41, 0x3A, 0x18, 0xEF, 0xB2, 0x1E, 0x9E, 0xBF, 0x6F, 0x8D, 0xDC, 0x6A, 0x9A, 0x51, 0x48, 0x6A, 0x1E, 0x19, 0x9E, 0xDB };

uint8_t g_rx_buff[HID_DATA_BYTESIZE] = { 0 };
uint8_t g_rx_size = 0;
volatile bool g_data_received = false;

volatile bool g_is_encrypted_key_started = false;
volatile bool g_is_all_keys_received = false;
unsigned char encrypted_key[SEALED_KEY_BYTESIZE] = { 0 };
unsigned char program_public_key[crypto_box_PUBLICKEYBYTES] = { 0 };

static inline void handle_encrypted_key(const char* data) {
	if (!g_is_encrypted_key_started) {
		memcpy(encrypted_key, &data[POCKET_FLAG_BYTESIZE], POCKET_DATA_BYTESIZE);
		g_is_encrypted_key_started = true;
	}
	else {
		memcpy(&encrypted_key[POCKET_DATA_BYTESIZE], &data[POCKET_FLAG_BYTESIZE], SEALED_KEY_BYTESIZE - POCKET_DATA_BYTESIZE);
		g_is_encrypted_key_started = false;
	}
}

static inline void handle_program_primary_key(const char* data) {
	memcpy(program_public_key, &data[POCKET_FLAG_BYTESIZE], crypto_box_PUBLICKEYBYTES);
	g_is_all_keys_received = true;
}

static inline void decrypt_and_seal_key(unsigned char* sealed_key_out) {
	unsigned char decrypted_key[DECRYPTED_KEY_BYTESIZE] = { 0 };
	if (crypto_box_seal_open(decrypted_key, encrypted_key, SEALED_KEY_BYTESIZE, DEVICE_PUBLIC_KEY, DEVICE_SECRET_KEY) != 0) {
		set_led_color(rgb_color(255, 0, 0));
	}

	crypto_box_seal(sealed_key_out, decrypted_key, DECRYPTED_KEY_BYTESIZE, program_public_key);
}

static inline void send_sealed_key(const unsigned char* sealed_key) {
	unsigned char first_part[HID_DATA_BYTESIZE] = { 0 };
	memcpy(first_part, sealed_key, HID_DATA_BYTESIZE);

	unsigned char second_part[HID_DATA_BYTESIZE] = { 0 };
	memcpy(second_part, &sealed_key[HID_DATA_BYTESIZE], SEALED_KEY_BYTESIZE - HID_DATA_BYTESIZE);

	while (!tud_hid_ready()) {
		tud_task();
	}

	tud_hid_report(0, first_part, HID_DATA_BYTESIZE);

	while (!tud_hid_ready()) {
		tud_task();
	}

	tud_hid_report(0, second_part, HID_DATA_BYTESIZE);
}

void handle_device_pk_request() {
	while (!tud_hid_ready()) {
		tud_task();
	}

	tud_hid_report(0, DEVICE_PUBLIC_KEY, HID_DATA_BYTESIZE);
}


static inline void handle_received_data() {
	if (!g_data_received) {
		return;
	}

	g_data_received = false;

	char data[HID_DATA_BYTESIZE] = { 0 };
	memcpy(data, g_rx_buff, g_rx_size);

	char flag = data[0];
	if (flag == FLAG_ENCRYPTED_KEY) {
		handle_encrypted_key(data);
	}
	else if (flag == FLAG_PROGRAM_PUBLIC_KEY) {
		handle_program_primary_key(data);
	} else if (flag == FLAG_DEVICE_PUBLIC_KEY_REQUEST) {
		handle_device_pk_request();
	}

	if (g_is_all_keys_received) {
		g_is_all_keys_received = false;

		unsigned char sealed_key[SEALED_KEY_BYTESIZE];
		decrypt_and_seal_key(sealed_key);
		send_sealed_key(sealed_key);
	}
}

int main() {
	init_led();

	tusb_rhport_init_t dev_init = {
		.role = TUSB_ROLE_DEVICE,
		.speed = TUSB_SPEED_AUTO
	};
	tusb_init(0, &dev_init);

	set_randombytes_implementation();
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

	memcpy(g_rx_buff, buffer, bufsize > HID_DATA_BYTESIZE ? HID_DATA_BYTESIZE : bufsize);
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
