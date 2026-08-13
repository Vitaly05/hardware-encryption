#include "hid.h"

#include <hidsdi.h>
#include <SetupAPI.h>
#include <vector>
#include <stdexcept>
#include <iostream>

#include "crypto_helper.h"

#pragma comment(lib, "hid.lib")
#pragma comment(lib, "setupapi.lib")

HANDLE HID::openDevice() {
    GUID hidGuid;
    HidD_GetHidGuid(&hidGuid);

    HDEVINFO deviceInfoSet = SetupDiGetClassDevsW(&hidGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        return INVALID_HANDLE_VALUE;
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
                        CloseHandle(hCheck);

                        hFinalDevice = CreateFileW(
                            deviceInterfaceDetailData->DevicePath,
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_FLAG_OVERLAPPED,
                            NULL
                        );
                        break;
                    }
                }
                CloseHandle(hCheck);
            }
        }
    }

    SetupDiDestroyDeviceInfoList(deviceInfoSet);
    return hFinalDevice;
}

void HID::closeHandles() {
    CloseHandle(overlappedWrite.hEvent);
    CloseHandle(overlappedRead.hEvent);
    CloseHandle(handle);
}

HID::HID(const WORD vid, const WORD pid) : vid(vid), pid(pid) {
    handle = openDevice();

    if (handle == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Device not found.");
    }

    overlappedWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    overlappedRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}

HID::~HID() {
    closeHandles();
}

bool g_is_encrypted_key_started = false;
int g_sended_key_iteration = 0;

void HID::Write(const BYTE* data) {
    BYTE buff[HID_DATA_BYTE_SIZE + 1] = { 0 };
    memcpy(&buff[1], data, HID_DATA_BYTE_SIZE);

    DWORD bytesWritten = 0;

    if (!WriteFile(handle, buff, sizeof(buff), &bytesWritten, &overlappedWrite)) {
        if (GetLastError() != ERROR_IO_PENDING) {
            throw std::runtime_error("Error while sending data to device.");
        }
    }

    WaitForSingleObject(overlappedWrite.hEvent, INFINITE);

    std::cout << "Sended to device" << std::endl;
}

std::array<BYTE, HID_DATA_BYTE_SIZE> HID::Read() {
    BYTE buff[HID_DATA_BYTE_SIZE + 1] = { 0 };
    DWORD bytesRead = 0;

    std::cout << "Start reading..." << std::endl;

    if (!ReadFile(handle, buff, sizeof(buff), &bytesRead, &overlappedRead)) {
        if (GetLastError() != ERROR_IO_PENDING) {
            throw std::runtime_error("Error while reading data from device.");
        }
    }

    DWORD waitResult = WaitForSingleObject(overlappedRead.hEvent, INFINITE);
    if (waitResult == WAIT_TIMEOUT) {
        throw std::runtime_error("No answer from device.");
    }

    GetOverlappedResult(handle, &overlappedRead, &bytesRead, TRUE);

    std::array<BYTE, HID_DATA_BYTE_SIZE> data;
    memcpy(data.data(), &buff[1], HID_DATA_BYTE_SIZE);

    return data;
}