#include "crypto_hid_controller.h"

#include <vector>
#include <iostream>

CryptoHidController::CryptoHidController(HID& hid) : hid(hid) {}

void CryptoHidController::send_keys(const BYTE* ek, int ek_size, const BYTE* ppk, int ppk_size, std::string& error_message_out) {
	std::vector<Pocket> pockets;

	collect_pockets(pockets, FLAG_ENCRYPTED_KEY, ek, ek_size);
	collect_pockets(pockets, FLAG_PROGRAM_PUBLIC_KEY, ppk, ppk_size);

	for (int i = 0; i < pockets.size(); i++) {
        hid.Write(pockets[i].data, error_message_out);
	}
}

void CryptoHidController::read_key(BYTE* key_out, std::string& error_message_out) {
    std::array<BYTE, HID_DATA_BYTESIZE> pocket1 = hid.Read(error_message_out);
	memcpy(key_out, pocket1.data(), 64);

    std::array<BYTE, HID_DATA_BYTESIZE> pocket2 = hid.Read(error_message_out);
	memcpy(&key_out[64], pocket2.data(), 80 - 64);
}

void CryptoHidController::collect_pockets(std::vector<Pocket>& pockets_out, const POCKET_FLAG flag, const BYTE* data, size_t data_size) {
	int pocket_id = 0;
	
	while (data_size > 0) {
		Pocket pocket;
		int index = POCKET_DATA_BYTESIZE * pocket_id;
		size_t size = data_size > POCKET_DATA_BYTESIZE ? POCKET_DATA_BYTESIZE : data_size;

		collect_pocket(pocket.data, flag, &data[index], size);
		pockets_out.push_back(pocket);
		pocket_id++;

		if (data_size >= POCKET_DATA_BYTESIZE) {
			data_size -= POCKET_DATA_BYTESIZE;
		}
		else {
			data_size = 0;
		}
	}
}


void CryptoHidController::collect_pocket(BYTE* data_out, const POCKET_FLAG flag, const BYTE* data, size_t data_size) {
	memcpy(data_out, &flag, POCKET_FLAG_BYTESIZE);
	memcpy(&data_out[POCKET_FLAG_BYTESIZE], data, data_size);
}