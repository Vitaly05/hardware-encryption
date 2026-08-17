#pragma once

#include <Windows.h>
#include <string>
#include <array>

#include "shared_config.h"
#include "sodium.h"
#include "crypto_helper.h"

class HID {
private:
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
	std::array<BYTE, HID_DATA_BYTESIZE> Read();
};