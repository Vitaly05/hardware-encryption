#include "crypto_hid_controller.h"

#include <vector>
#include <locale>
#include <codecvt>

CryptoHidController::CryptoHidController(HID& hid) : hid(hid) {}

void CryptoHidController::send_keys(const BYTE* ek, int ek_size, const BYTE* ppk, int ppk_size, std::string& error_message_out) {
	std::vector<Pocket> pockets;

	collect_pockets(pockets, FLAG_ENCRYPTED_KEY, ek, ek_size);
	collect_pockets(pockets, FLAG_PROGRAM_PUBLIC_KEY, ppk, ppk_size);

	for (int i = 0; i < pockets.size(); i++) {
        hid.Write(pockets[i].data, hid_serial, error_message_out);
	}
}

void CryptoHidController::set_serial(const std::string serial) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    hid_serial = converter.from_bytes(serial);
}

void CryptoHidController::read_key(BYTE* key_out, std::string& error_message_out) {
    std::array<BYTE, HID_DATA_BYTESIZE> pocket1 = hid.Read(hid_serial, error_message_out);
    memcpy(key_out, pocket1.data(), HID_DATA_BYTESIZE);

    std::array<BYTE, HID_DATA_BYTESIZE> pocket2 = hid.Read(hid_serial, error_message_out);
    memcpy(&key_out[HID_DATA_BYTESIZE], pocket2.data(), SEALED_KEY_BYTESIZE - HID_DATA_BYTESIZE);
}

void CryptoHidController::get_public_key(BYTE* device_public_key_out, std::string& error_message_out) {
    BYTE request[HID_DATA_BYTESIZE] = { 0 };
    const POCKET_FLAG flag = FLAG_DEVICE_PUBLIC_KEY_REQUEST;
    memcpy(request, &flag, POCKET_FLAG_BYTESIZE);

    hid.Write(request, hid_serial, error_message_out);

    std::array<BYTE, HID_DATA_BYTESIZE> answer = hid.Read(hid_serial, error_message_out);
    memcpy(device_public_key_out, answer.data(), crypto_box_PUBLICKEYBYTES);
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