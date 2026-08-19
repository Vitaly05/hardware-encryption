#pragma once

extern "C" {
    #include <windows.h>
}
#include <string>
#include <array>
#include <vector>

#include "shared_config.h"
#include "sodium.h"
#include "crypto_helper.h"
#include "hid_device_registry.h"

class HID {
private:
	WORD vid; WORD pid;
    HidDeviceRegistry hid_device_registry;
	OVERLAPPED overlappedWrite = { 0 };
	OVERLAPPED overlappedRead = { 0 };

    void openDevices();
	void closeHandles();

public:
	HID(const HID&) = delete;
	HID(HID&&) = delete;
	HID(const WORD vid, const WORD pid);
	~HID();

    void Write(const BYTE* data, std::wstring device_serial, std::string& error_message_out);
    std::array<BYTE, HID_DATA_BYTESIZE> Read(std::wstring device_serial, std::string& error_message_out);
    std::vector<std::wstring> GetDeviceSerials();
    void RefreshDevices();
};