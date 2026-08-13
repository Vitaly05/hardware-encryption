#include <iostream>
#include <string>
#include <Windows.h>
#include <stdexcept>
#include <vector>
#include <array>

#include "hid.h"
#include "crypto_helper.h"
#include "encrypted_file.h"
#include "crypto_hid_controller.h"
#include "sodium.h"

#pragma comment(lib, "libsodium.lib")

const WORD VID = 0x2E8A;
const WORD PID = 0x464A;

const unsigned char DEVICE_PUBLIC_KEY[] = { 0x27, 0x56, 0x30, 0x1B, 0xF0, 0x3B, 0xFA, 0x28, 0xEB, 0xD1, 0x35, 0xFF, 0x48, 0xF2, 0xC5, 0x54, 0x6C, 0x98, 0x8C, 0x4C, 0x1D, 0x61, 0xA2, 0xB4, 0x07, 0x99, 0x4A, 0x40, 0xDB, 0x97, 0x31, 0x0B };

std::string getString() {
    return "Hello!";

    std::cout << "Enter text: ";

    std::string a;
    std::cin >> a;

    return a;
}

int main() {
    if (sodium_init() < 0) {
        return 1;
    }

    std::string message = "Test from file";
    EncryptedData ed = encrypt_data(message, DEVICE_PUBLIC_KEY);

    if (save_file("test", ed) != 0) {
        std::cout << "Error while saving file." << std::endl;
        return 1;
    }

    EncryptedData edf;

    if (read_file("test", &edf) != 0) {
        std::cout << "Error while reading file." << std::endl;
        return 1;
    }

    unsigned char pk[crypto_box_PUBLICKEYBYTES];
    unsigned char sk[crypto_box_SECRETKEYBYTES];
    crypto_box_keypair(pk, sk);

    try {
        HID hid(VID, PID);
        CryptoHidController chc(hid);

        chc.send_keys(edf.encrypted_key, EF_KEY_BYTE_SIZE, pk, crypto_box_PUBLICKEYBYTES);

        unsigned char readed_key[CH_KEY_BYTE_SIZE];
        chc.read_key(readed_key);

        unsigned char decrypted_key[crypto_aead_aegis256_KEYBYTES];

        if (crypto_box_seal_open(decrypted_key, readed_key, CH_KEY_BYTE_SIZE, pk, sk) != 0) {
            std::cerr << "Message corrupted or not intended for this recipient." << std::endl;
            return 1;
        }

        std::string decrypted = decrypt_data(edf, decrypted_key);

        std::cout << "Decrypted: " << decrypted << std::endl;
    }
    catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        return 1;
    }

    return 0;
}
