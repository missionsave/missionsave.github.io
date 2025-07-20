// g++ -std=c++17 -o brightnessctl brightnessctl.cpp

#include <fstream>
#include <iostream>
#include <cstdio>
#include <string>
#include <memory>
#include <stdexcept>
#include <array>
#include <thread>
#include <iostream>
#include <cstdio>
#include <string>
#include <memory>
#include <vector>
#include <algorithm>

#include <iostream>
#include <cstdio>
#include <string>
#include <memory>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

class BrightnessManager {
public:
    BrightnessManager() {
        // Find first available backlight interface
        backlight_path_ = find_backlight_path();
        if (backlight_path_.empty()) {
            throw std::runtime_error("No backlight interface found in /sys/class/backlight/");
        }
        
        max_brightness_ = read_int(backlight_path_ + "/max_brightness");
        std::cout << "Using backlight interface: " << backlight_path_ << "\n";
    }

    void adjust(int delta) {
        int current = read_int(backlight_path_ + "/brightness");
        int new_value = current + delta;
        new_value = std::clamp(new_value, 10, max_brightness_);
        
        write_int(backlight_path_ + "/brightness", new_value);
    }

private:
    std::string backlight_path_;
    int max_brightness_;

    static std::string find_backlight_path() {
        const std::string base_path = "/sys/class/backlight/";
        
        try {
            for (const auto& entry : fs::directory_iterator(base_path)) {
                if (entry.is_directory()) {
                    std::string path = entry.path().string();
                    // Check if brightness file exists (don't check writability)
                    if (fs::exists(path + "/brightness") && 
                        fs::exists(path + "/max_brightness")) {
                        return path;
                    }
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << "\n";
        }
        return "";
    }

    static int read_int(const std::string& path) {
        std::ifstream file(path);
        if (!file) throw std::runtime_error("Can't read " + path);
        int value;
        file >> value;
        return value;
    }

    static void write_int(const std::string& path, int value) {
        std::ofstream file(path);
        if (!file) throw std::runtime_error("Can't write " + path);
        file << value;
    }
};

// ... rest of the code remains the same as previous version ...

void handle_acpi_events(BrightnessManager& manager) {
    constexpr int step = 250;
    std::array<char, 256> buffer;

    // Using popen with automatic cleanup
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("acpi_listen", "r"), pclose);
    if (!pipe) throw std::runtime_error("Failed to start acpi_listen");

    while (fgets(buffer.data(), buffer.size(), pipe.get())) {
        std::string event(buffer.data());
        
        if (event.find("brightnessup") != std::string::npos) {
            manager.adjust(step);
        }
        else if (event.find("brightnessdown") != std::string::npos) {
            manager.adjust(-step);
        }
    }
}

int main() {
    try {
        BrightnessManager manager;
        std::cout << "Listening for ACPI brightness events...\n";
        handle_acpi_events(manager);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        
        // Helpful troubleshooting tips
        std::cerr << "\nTroubleshooting:\n";
        std::cerr << "1. Ensure acpi_listen is installed (usually in acpid package)\n";
        std::cerr << "2. Check your user has write access to:\n";
        std::cerr << "   - /sys/class/backlight/*/brightness\n";
        std::cerr << "3. Verify acpid service is running\n";
        
        return 1;
    }
    return 0;
}