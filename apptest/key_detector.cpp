// sudo apt install libevdev-dev
// g++ -std=c++17 key_detector.cpp -o key_detector -levdev -lstdc++fs


// needs sudo

#include <libevdev-1.0/libevdev/libevdev.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <fcntl.h>
#include <unistd.h> 

namespace fs = std::filesystem;

class BrightnessController {
private:
    std::string backlight_path;

public:
    BrightnessController() {
        // Auto-detect backlight interface
        for (const auto& entry : fs::directory_iterator("/sys/class/backlight/")) {
            backlight_path = entry.path();
            break; // Use first found device
        }
        if (backlight_path.empty()) {
            throw std::runtime_error("No backlight device found");
        }
    }

    int get_current() {
        std::ifstream brightness_file(backlight_path + "/brightness");
        int current;
        brightness_file >> current;
        return current;
    }

    int get_max() {
        std::ifstream max_file(backlight_path + "/max_brightness");
        int max;
        max_file >> max;
        return max;
    }

    void set_brightness(int value) {
        value = std::clamp(value, 0, get_max());
        std::ofstream(backlight_path + "/brightness") << value;
    }

    void adjust(int percent) {
        int step = (get_max() * percent) / 100;
        set_brightness(get_current() + step);
    }
};

class KeyListener {
    BrightnessController bc;
    std::vector<int> fds;
    std::vector<libevdev*> devices;

public:
    KeyListener() {
        // Find all input devices
        for (const auto& entry : fs::directory_iterator("/dev/input/")) {
            std::string path = entry.path();
            if (path.find("event") != std::string::npos) {
                int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
                if (fd < 0) continue;

                libevdev* dev = nullptr;
                if (libevdev_new_from_fd(fd, &dev) == 0) {
                    if (has_brightness_keys(dev)) {
                        fds.push_back(fd);
                        devices.push_back(dev);
                    } else {
                        libevdev_free(dev);
                        close(fd);
                    }
                }
            }
        }
    }

    ~KeyListener() {
        for (size_t i = 0; i < devices.size(); i++) {
            libevdev_free(devices[i]);
            close(fds[i]);
        }
    }

    bool has_brightness_keys(libevdev* dev) {
        return libevdev_has_event_code(dev, EV_KEY, KEY_BRIGHTNESSUP) ||
               libevdev_has_event_code(dev, EV_KEY, KEY_BRIGHTNESSDOWN) ||
               libevdev_has_event_code(dev, EV_KEY, KEY_VOLUMEUP) || 
               libevdev_has_event_code(dev, EV_KEY, KEY_VOLUMEDOWN);
    }

    void listen() {
        std::cout << "Listening for brightness keys... (Press Ctrl+C to exit)\n";
        
        while (true) {
            for (size_t i = 0; i < devices.size(); i++) {
                input_event ev;
                int rc = LIBEVDEV_READ_STATUS_SUCCESS;

                while (rc == LIBEVDEV_READ_STATUS_SUCCESS) {
                    rc = libevdev_next_event(devices[i], LIBEVDEV_READ_FLAG_NORMAL, &ev);
                    
                    if (rc == LIBEVDEV_READ_STATUS_SUCCESS && ev.type == EV_KEY && ev.value == 1) {
                        handle_key(ev.code);
                    }
                }
            }
            usleep(100000); // 10ms delay to prevent CPU overload
        }
    }

    void handle_key(int keycode) {
        switch (keycode) {
            case KEY_BRIGHTNESSUP:
            case KEY_VOLUMEUP:
                bc.adjust(5);
                std::cout << "Brightness increased\n";
                break;
                
            case KEY_BRIGHTNESSDOWN:
            case KEY_VOLUMEDOWN:
                bc.adjust(-5);
                std::cout << "Brightness decreased\n";
                break;
        }
    }
};


int main(int argc, char* argv[]) {
	if (geteuid() != 0) {
        std::cout << "Relaunching as root...\n";

        std::string command = "sudo ";
        for (int i = 0; i < argc; ++i) {
            command += argv[i];
            command += " ";
        }

        system(command.c_str());
        return 0;
    }
    std::cout << "Running with root privileges.\n";

    try {
        KeyListener listener;
        listener.listen();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}