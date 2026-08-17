#pragma once

#include <string>

#include "shared_config.h"
#include "crypto_helper.h"

constexpr auto EF_KEY_BYTE_SIZE = SEALED_KEY_BYTESIZE;
constexpr auto EF_NONCE_BYTE_SIZE = crypto_aead_aegis256_NPUBBYTES;
constexpr auto EF_DATA_SIZE_BYTE_SIZE = sizeof(unsigned long long);
constexpr auto EF_EXTENSION = ".lhef";

int read_file(std::string path, EncryptedData* ed_out);
int save_file(std::string path, const EncryptedData& encrypted_data);