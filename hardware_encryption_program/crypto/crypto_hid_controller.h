#pragma once

#include <Windows.h>
#include <vector>
#include <string>

#include "shared_config.h"
#include "hid.h"

struct Pocket {
	BYTE data[HID_DATA_BYTESIZE];

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
    void send_keys(const BYTE* encrypted_key, int encrypted_key_size, const BYTE* program_public_key, int program_public_key_size, std::string& error_message_out);
    void read_key(BYTE* key_out, std::string& error_message_out);
    void get_public_key(BYTE* device_public_key_out, std::string& error_message_out);
};
