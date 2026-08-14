#include "crypto_helper.h"

#include <stdexcept>

EncryptedData encrypt_data(const std::string& data, const unsigned char* pk) {
    unsigned char key[crypto_aead_aegis256_KEYBYTES] = { 0 };
    EncryptedData d;

    d.encrypted.resize(data.size() + crypto_aead_aegis256_ABYTES);

    crypto_aead_aegis256_keygen(key);
    randombytes_buf(d.nonce, sizeof(d.nonce));

    crypto_aead_aegis256_encrypt(d.encrypted.data(), &d.encrypted_len,
        reinterpret_cast<const unsigned char*>(data.data()), data.size(),
        NULL, 0,
        NULL, d.nonce, key
    );

    crypto_box_seal(d.encrypted_key, key, crypto_aead_aegis256_KEYBYTES, pk);

    return d;
}

std::string decrypt_data(const EncryptedData& ed, const unsigned char* key) {
    std::vector<unsigned char> decrypted(ed.encrypted_len - crypto_aead_aegis256_ABYTES);
    unsigned long long decrypted_len;

    int res = crypto_aead_aegis256_decrypt(decrypted.data(), &decrypted_len,
        NULL,
        ed.encrypted.data(), ed.encrypted_len,
        NULL, 0,
        ed.nonce, key
    );

    if (res != 0) {
        throw std::runtime_error("Data forged!");
    }

    return std::string(decrypted.begin(), decrypted.begin() + decrypted_len);
}