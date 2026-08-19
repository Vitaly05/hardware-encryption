#include "hid.h"

extern "C" {
    #include <windows.h>
    #include <setupapi.h>
    #include <hidsdi.h>
}
#include <vector>
#include <stdexcept>
#include <iostream>
#include <codecvt>
#include <locale>

#include "crypto_helper.h"

#pragma comment(lib, "hid.lib")
#pragma comment(lib, "setupapi.lib")

void HID::openDevices() {
    HidDeviceRegistry registry;

    GUID hidGuid;
    HidD_GetHidGuid(&hidGuid);

    HDEVINFO deviceInfoSet = SetupDiGetClassDevsW(&hidGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        hid_device_registry = std::move(registry);
    }

    SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
    deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    DWORD memberIndex = 0;
    HANDLE hFinalDevice = INVALID_HANDLE_VALUE;

    while (SetupDiEnumDeviceInterfaces(deviceInfoSet, NULL, &hidGuid, memberIndex, &deviceInterfaceData)) {
        memberIndex++;

        DWORD detailSize = 0;
        SetupDiGetDeviceInterfaceDetailW(deviceInfoSet, &deviceInterfaceData, NULL, 0, &detailSize, NULL);

        std::vector<BYTE> detailBuffer(detailSize);
        PSP_DEVICE_INTERFACE_DETAIL_DATA_W deviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA_W)detailBuffer.data();
        deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);

        if (SetupDiGetDeviceInterfaceDetailW(deviceInfoSet, &deviceInterfaceData, deviceInterfaceDetailData, detailSize, NULL, NULL)) {
            HANDLE hCheck = CreateFileW(
                deviceInterfaceDetailData->DevicePath,
                0,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL
            );

            if (hCheck != INVALID_HANDLE_VALUE) {
                HIDD_ATTRIBUTES attributes;
                attributes.Size = sizeof(HIDD_ATTRIBUTES);

                if (HidD_GetAttributes(hCheck, &attributes)) {
                    if (attributes.VendorID == this->vid && attributes.ProductID == this->pid) {
                        wchar_t serialBuffer[126] = { 0 };
                        std::wstring serialNumber = L"UNKNOWN_SERIAL_" + std::to_wstring(memberIndex); // Фолбэк, если серийника нет

                        if (HidD_GetSerialNumberString(hCheck, serialBuffer, sizeof(serialBuffer))) {
                            serialNumber = serialBuffer;
                        }

                        CloseHandle(hCheck);
                        hCheck = INVALID_HANDLE_VALUE;

                        hFinalDevice = CreateFileW(
                            deviceInterfaceDetailData->DevicePath,
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_FLAG_OVERLAPPED,
                            NULL
                        );

                        if (hFinalDevice != INVALID_HANDLE_VALUE) {
                            registry.devices[serialNumber] = hFinalDevice;
                        }
                    }
                }

                if (hCheck != INVALID_HANDLE_VALUE) {
                    CloseHandle(hCheck);
                }
            }
        }
    }

    SetupDiDestroyDeviceInfoList(deviceInfoSet);

    hid_device_registry = std::move(registry);
}

void HID::closeHandles() {
    CloseHandle(overlappedWrite.hEvent);
    CloseHandle(overlappedRead.hEvent);
}

HID::HID(const WORD vid, const WORD pid) : vid(vid), pid(pid) {
    openDevices();

    overlappedWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    overlappedRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}

HID::~HID() {
    closeHandles();
}

void HID::Write(const BYTE* data, std::wstring device_serial, std::string& error_message_out) {
    auto it = hid_device_registry.devices.find(device_serial);

    if (it == hid_device_registry.devices.end()) {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>converter;
        std::string device_serial_utf8 = converter.to_bytes(device_serial);
        error_message_out = "Device with serial number '" + device_serial_utf8 + "' was not found";

        return;
    }

    HANDLE handle = it->second;
    BYTE buff[HID_DATA_BYTESIZE + 1] = { 0 };
    memcpy(&buff[1], data, HID_DATA_BYTESIZE);

    DWORD bytesWritten = 0;

    if (!WriteFile(handle, buff, sizeof(buff), &bytesWritten, &overlappedWrite)) {
        DWORD err = GetLastError();

        if (err != ERROR_IO_PENDING) {
            if (err == ERROR_DEVICE_NOT_CONNECTED || err == ERROR_GEN_FAILURE) {
                error_message_out = "Device was disconnected";
                CloseHandle(handle);
                hid_device_registry.devices.erase(it);

                return;
            }

            error_message_out = "Error while sending data to device. Code: " + std::to_string(err);

            return;
        }
    }

    WaitForSingleObject(overlappedWrite.hEvent, INFINITE);

    std::cout << "Sended to device" << std::endl;
}

std::array<BYTE, HID_DATA_BYTESIZE> HID::Read(std::wstring device_serial, std::string& error_message_out) {
    error_message_out = "";
    std::array<BYTE, HID_DATA_BYTESIZE> data;

    auto it = hid_device_registry.devices.find(device_serial);

    if (it == hid_device_registry.devices.end()) {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::string device_serial_utf8 = converter.to_bytes(device_serial);
        error_message_out = "Device with serial number '" + device_serial_utf8 + "' was not found";

        return data;
    }

    HANDLE handle = it->second;
    BYTE buff[HID_DATA_BYTESIZE + 1] = { 0 };
    DWORD bytesRead = 0;

    std::cout << "Start reading..." << std::endl;

    if (!ReadFile(handle, buff, sizeof(buff), &bytesRead, &overlappedRead)) {
        DWORD err = GetLastError();

        if (err != ERROR_IO_PENDING) {
            if (err == ERROR_DEVICE_NOT_CONNECTED || err == ERROR_GEN_FAILURE) {
                error_message_out = "Device was disconnected";
                CloseHandle(handle);
                hid_device_registry.devices.erase(it);

                return data;
            }

            error_message_out = "Error while reading data from device. Code: " + std::to_string(err);

            return data;
        }
    }

    DWORD waitResult = WaitForSingleObject(overlappedRead.hEvent, 5000);
    if (waitResult == WAIT_TIMEOUT) {
        CancelIoEx(handle, &overlappedRead);
        error_message_out = "No answer from device (Timeout)";

        return data;
    }

    if (!GetOverlappedResult(handle, &overlappedRead, &bytesRead, FALSE)) {
        DWORD err = GetLastError();

        if (err == ERROR_DEVICE_NOT_CONNECTED || err == ERROR_GEN_FAILURE || err == ERROR_OPERATION_ABORTED) {
            error_message_out = "Device was disconnected during reading";
            CloseHandle(handle);
            hid_device_registry.devices.erase(it);

            return data;
        }

        error_message_out = "Asynchronous read failed. Code: " + std::to_string(err);

        return data;
    }

    memcpy(data.data(), &buff[1], HID_DATA_BYTESIZE);

    return data;
}

std::vector<std::wstring> HID::GetDeviceSerials() {
    return hid_device_registry.getAvailableSerials();
}

void HID::RefreshDevices() {
    openDevices();
}
