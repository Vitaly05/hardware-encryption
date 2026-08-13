#pragma once

#include <Windows.h>
#include <vector>

#include "hid.h"

typedef enum {
	FLAG_ENCRYPTED_KEY = 1,
	FLAG_PROGRAM_PUBLIC_KEY = 2,
} POCKET_FLAG;

static struct Pocket {
	BYTE data[HID_DATA_BYTE_SIZE];

	Pocket() : data{} {}
};

class CryptoHidController {
private:
	HID& hid;

	void collect_pockets(std::vector<Pocket>& pockets_out, const POCKET_FLAG flag, const BYTE* data, size_t data_size);
	void collect_pocket(BYTE* data_out, const POCKET_FLAG flag, const BYTE* data, size_t data_size);

public:
	CryptoHidController(const CryptoHidController&) = delete;
	CryptoHidController(CryptoHidController&&) = delete;
	CryptoHidController(HID& hid);
	void send_keys(const BYTE* encrypted_key, int encrypted_key_size, const BYTE* program_public_key, int program_public_key_size);
	void read_key(BYTE* key_out);
};
