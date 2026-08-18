#include "encrypted_file.h"

#include <iostream>
#include <fstream>
#include <vector>

int read_file(std::string path, EncryptedData* ed_out) {
    std::ifstream file(path, std::ios::in | std::ios::binary);

	if (!file.is_open()) {
		return 1;
	}

	file.read(reinterpret_cast<char*>(ed_out->encrypted_key), EF_KEY_BYTE_SIZE);
	file.read(reinterpret_cast<char*>(ed_out->nonce), EF_NONCE_BYTE_SIZE);
	file.read(reinterpret_cast<char*>(&ed_out->encrypted_len), EF_DATA_SIZE_BYTE_SIZE);
	
	ed_out->encrypted.resize(ed_out->encrypted_len);
	file.read(reinterpret_cast<char*>(ed_out->encrypted.data()), ed_out->encrypted_len);

	file.close();

	return 0;
}

int save_file(std::string path, const EncryptedData& ed) {
    std::ofstream file(path, std::ios::out | std::ios::binary);
	
	if (!file.is_open()) {
		return 1;
	}

	file.write(reinterpret_cast<const char*>(ed.encrypted_key), EF_KEY_BYTE_SIZE);
	file.write(reinterpret_cast<const char*>(ed.nonce), EF_NONCE_BYTE_SIZE);
	file.write(reinterpret_cast<const char*>(&ed.encrypted_len), EF_DATA_SIZE_BYTE_SIZE);
	file.write(reinterpret_cast<const char*>(ed.encrypted.data()), ed.encrypted_len);

	file.close();

	return 0;
}