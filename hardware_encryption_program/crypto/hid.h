#pragma once

extern "C" {
    #include <windows.h>
}
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

    void Write(const BYTE* data, std::string& error_message_out);
    std::array<BYTE, HID_DATA_BYTESIZE> Read(std::string& error_message_out);
};