
// sudo apt install libasound2-dev acpid
// g++ -std=c++17 -o fnhandle fnhandle.cpp -lasound 

#include <algorithm>
#include <iostream>
#include <cstdio>
#include <string>
#include <memory>
#include <filesystem>
#include <vector>
#include <array>
#include <fstream>
#include <unistd.h>
#include <alsa/asoundlib.h> // For audio control

namespace fs = std::filesystem;

// ==================== Brightness Control ====================
class BrightnessManager {
public:
    BrightnessManager() {
        backlight_path_ = find_backlight_path();
        if (backlight_path_.empty()) {
            throw std::runtime_error("No backlight interface found");
        }
        max_brightness_ = read_int(backlight_path_ + "/max_brightness");
        std::cout << "Using backlight: " << backlight_path_ << "\n";
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
                    if (fs::exists(path + "/brightness") && 
                        fs::exists(path + "/max_brightness")) {
                        return path;
                    }
                }
            }
        } catch (const fs::filesystem_error&) {}
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

// ==================== Audio Control ====================
class AudioManager {
public:
    AudioManager() {
        if (snd_mixer_open(&mixer_handle_, 0) != 0) {
            throw std::runtime_error("Couldn't open ALSA mixer");
        }
        if (snd_mixer_attach(mixer_handle_, "default") != 0) {
            snd_mixer_close(mixer_handle_);
            throw std::runtime_error("Couldn't attach to default mixer");
        }
        if (snd_mixer_selem_register(mixer_handle_, nullptr, nullptr) != 0) {
            snd_mixer_close(mixer_handle_);
            throw std::runtime_error("Mixer registration failed");
        }
        if (snd_mixer_load(mixer_handle_) != 0) {
            snd_mixer_close(mixer_handle_);
            throw std::runtime_error("Couldn't load mixer controls");
        }

        // Find master playback volume control
        snd_mixer_selem_id_alloca(&sid_);
        snd_mixer_selem_id_set_index(sid_, 0);
        snd_mixer_selem_id_set_name(sid_, "Master");
        elem_ = snd_mixer_find_selem(mixer_handle_, sid_);
        if (!elem_) {
            snd_mixer_close(mixer_handle_);
            throw std::runtime_error("Couldn't find master control");
        }
    }

    ~AudioManager() {
        if (mixer_handle_) snd_mixer_close(mixer_handle_);
    }

    void adjust_volume(int delta) {
        long min, max, current;
        snd_mixer_selem_get_playback_volume_range(elem_, &min, &max);
        snd_mixer_selem_get_playback_volume(elem_, SND_MIXER_SCHN_FRONT_LEFT, &current);
        
        long new_vol = current + (delta * max / 100); // Delta as percentage of max
        new_vol = std::clamp(new_vol, min, max);
        
        snd_mixer_selem_set_playback_volume_all(elem_, new_vol);
    }

    void toggle_mute() {
        int mute;
        snd_mixer_selem_get_playback_switch(elem_, SND_MIXER_SCHN_FRONT_LEFT, &mute);
        snd_mixer_selem_set_playback_switch_all(elem_, !mute);
    }

private:
    snd_mixer_t* mixer_handle_ = nullptr;
    snd_mixer_selem_id_t* sid_ = nullptr;
    snd_mixer_elem_t* elem_ = nullptr;
};

// ==================== ACPI Event Handler ====================
void handle_acpi_events(BrightnessManager& brightness, AudioManager& audio) {
    constexpr int brightness_step = 50;
    constexpr int volume_step = 5; // 5% volume change per key press
    
    std::array<char, 256> buffer;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("acpi_listen", "r"), pclose);
    if (!pipe) throw std::runtime_error("Failed to start acpi_listen");

    std::cout << "Listening for ACPI events...\n";
    while (fgets(buffer.data(), buffer.size(), pipe.get())) {
        std::string event(buffer.data());
        
        // Brightness control
        if (event.find("video/brightnessup") != std::string::npos) {
            brightness.adjust(brightness_step);
            std::cout << "Brightness increased\n";
        }
        else if (event.find("video/brightnessdown") != std::string::npos) {
            brightness.adjust(-brightness_step);
            std::cout << "Brightness decreased\n";
        }
        // Volume control
        else if (event.find("button/volumeup") != std::string::npos) {
            audio.adjust_volume(volume_step);
            std::cout << "Volume increased\n";
        }
        else if (event.find("button/volumedown") != std::string::npos) {
            audio.adjust_volume(-volume_step);
            std::cout << "Volume decreased\n";
        }
        else if (event.find("button/mute") != std::string::npos) {
            audio.toggle_mute();
            std::cout << "Mute toggled\n";
        }
    }
}

int main() {
    try {
        BrightnessManager brightness;
        AudioManager audio;
        handle_acpi_events(brightness, audio);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << "\n";
        
        std::cerr << "\nTroubleshooting:\n";
        std::cerr << "1. Install required packages:\n";
        std::cerr << "   - acpid (for acpi_listen)\n";
        std::cerr << "   - libasound2-dev (for ALSA development files)\n";
        std::cerr << "2. Check your user is in audio/video groups\n";
        std::cerr << "3. Verify acpid service is running\n";
        
        return 1;
    }
    return 0;
}