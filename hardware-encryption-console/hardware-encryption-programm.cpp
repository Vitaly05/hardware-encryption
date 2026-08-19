#include <iostream>
#include <fstream>
#include <iomanip>

#include "sodium.h"

#pragma comment(lib, "libsodium.lib")

bool saveKeyBinary(const std::string& filename, const unsigned char* key, int size, std::string& error_message_out) {
    error_message_out = "";

    std::ofstream file(filename, std::ios::out);

    if (!file.is_open()) {
        error_message_out = "Failed to open or create file: " + filename;
        return false;
    }

    file << std::hex << std::uppercase;

    for (int i = 0; i < size; ++i) {
        file << "0x" << std::setfill('0') << std::setw(2) << static_cast<int>(key[i]);

        if (i < size - 1) {
            file << ", ";
        }
    }

    if (!file.good()) {
        error_message_out = "Error occurred while writing text data to file: " + filename;
        return false;
    }

    return true;
}

int main() {
    if (sodium_init() < 0) {
        return 1;
    }

    unsigned char pk[crypto_box_PUBLICKEYBYTES];
    unsigned char sk[crypto_box_SECRETKEYBYTES];
    crypto_box_keypair(pk, sk);

    std::string error_msg;

    if (!saveKeyBinary("public_key.lkey", pk, crypto_box_PUBLICKEYBYTES, error_msg)) {
        std::cerr << "Error: " << error_msg << std::endl;
    }
    else {
        std::cout << "Key saved successfully!" << std::endl;
    }

    if (!saveKeyBinary("secret_key.lkey", sk, crypto_box_SECRETKEYBYTES, error_msg)) {
        std::cerr << "Error: " << error_msg << std::endl;
    }
    else {
        std::cout << "Key saved successfully!" << std::endl;
    }

    return 0;
}
