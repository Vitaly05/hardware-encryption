#ifndef HID_DEVICE_REGISTRY_H
#define HID_DEVICE_REGISTRY_H

extern "C" {
    #include <windows.h>
    #include <setupapi.h>
    #include <hidsdi.h>
}
#include <string>
#include <vector>
#include <unordered_map>

struct HidDeviceRegistry {
    std::unordered_map<std::wstring, HANDLE> devices;

    ~HidDeviceRegistry() {
        for (auto& pair : devices) {
            if (pair.second != INVALID_HANDLE_VALUE) {
                CloseHandle(pair.second);
            }
        }
    }

    std::vector<std::wstring> getAvailableSerials() const {
        std::vector<std::wstring> serials;

        serials.reserve(devices.size());

        for (const auto& pair : devices) {
            serials.push_back(pair.first);
        }

        return serials;
    }

    HidDeviceRegistry() = default;
    HidDeviceRegistry(const HidDeviceRegistry&) = delete;
    HidDeviceRegistry& operator=(const HidDeviceRegistry&) = delete;

    HidDeviceRegistry(HidDeviceRegistry&&) noexcept = default;
    HidDeviceRegistry& operator=(HidDeviceRegistry&&) noexcept = default;
};


#endif // HID_DEVICE_REGISTRY_H
