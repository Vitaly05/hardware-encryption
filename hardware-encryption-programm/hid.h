#pragma once

#include <Windows.h>
#include <string>
#include <array>

#include "sodium.h"
#include "crypto_helper.h"

constexpr auto HID_DATA_BYTE_SIZE = 64;

class HID {
private:
	unsigned char encrypted_key[CH_KEY_BYTE_SIZE] = { 0 };
	unsigned char program_public_key[crypto_box_PUBLICKEYBYTES] = { 0 };
	unsigned char key_to_send[crypto_aead_aegis256_KEYBYTES] = { 0 };

	WORD vid; WORD pid;
	HANDLE handle;
	OVERLAPPED overlappedWrite = { 0 };
	OVERLAPPED overlappedRead = { 0 };

	HANDLE openDevice();
	void closeHandles();

public:
	HID(const HID&) = delete;
	HID(HID&&) = delete;
	HID(const WORD vid, const WORD pid);
	~HID();

	void Write(const BYTE* data);
	std::array<BYTE, HID_DATA_BYTE_SIZE> Read();
};