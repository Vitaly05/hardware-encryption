#pragma once

#include <string>
#include <vector>

#include "shared_config.h"
#include "sodium.h"

struct EncryptedData {
	std::vector<unsigned char> encrypted = { 0 };
	unsigned long long encrypted_len = 0;
	unsigned char nonce[crypto_aead_aegis256_NPUBBYTES] = { 0 };
	unsigned char encrypted_key[SEALED_KEY_BYTESIZE] = { 0 };
};

EncryptedData encrypt_data(const std::string& data, const unsigned char* primary_key);
std::string decrypt_data(const EncryptedData& encrypted_data, const unsigned char* key);
