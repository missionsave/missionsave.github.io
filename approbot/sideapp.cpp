#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <csignal>
#include <cstring>
#include <chrono>
#include <thread>

const char* files[2] = { "data_a.bin", "data_b.bin" };
const char* outputFile = "final_state.bin";
const size_t VECTOR_SIZE = 256;
const size_t FILESIZE = sizeof(uint64_t) + sizeof(float) * VECTOR_SIZE;

struct MappedData {
    uint64_t timestamp;
    float values[VECTOR_SIZE];
};

bool running = true;

// Handle Ctrl+C gracefully
void handleExit(int) {
    running = false;
}

// Check if main app is still alive
bool isRunning(pid_t pid) {
    return (kill(pid, 0) == 0);
}

// Create empty memory-mapped files if they don't exist
void createMemoryFiles() {
    for (int i = 0; i < 2; ++i) {
        std::ifstream test(files[i], std::ios::binary);
        if (test.good()) continue; // already exists

        int fd = open(files[i], O_RDWR | O_CREAT | O_TRUNC, 0666);
        if (fd < 0) {
            perror(("open " + std::string(files[i])).c_str());
            continue;
        }

        ftruncate(fd, FILESIZE);
        void* map = mmap(nullptr, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (map != MAP_FAILED) {
            std::memset(map, 0, FILESIZE); // zero out
            msync(map, FILESIZE, MS_SYNC);
            munmap(map, FILESIZE);
            std::cout << "📦 Created: " << files[i] << "\n";
        } else {
            perror(("mmap " + std::string(files[i])).c_str());
        }
        close(fd);
    }
}

// Read mapped memory into local struct
MappedData readMapped(const char* filename) {
    MappedData data{};
    int fd = open(filename, O_RDONLY);
    if (fd < 0) return data;

    void* map = mmap(nullptr, FILESIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (map != MAP_FAILED) {
        std::memcpy(&data, map, FILESIZE);
        munmap(map, FILESIZE);
    }
    close(fd);
    return data;
}

// Write selected memory to final output file
void writeFinal(const MappedData& data) {
    std::ofstream out(outputFile, std::ios::binary | std::ios::trunc);
    out.write(reinterpret_cast<const char*>(&data), sizeof(data));
    out.close();
    std::cout << "✅ Flushed to disk: timestamp = " << data.timestamp << "\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: sideapp <main_app_pid>\n";
        return 1;
    }

    pid_t mainPid = std::stoi(argv[1]);
    std::signal(SIGINT, handleExit);

    createMemoryFiles();

    std::cout << "🧭 Side app running. Watching PID " << mainPid << "...\n";

    while (running && isRunning(mainPid)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::cout << "💥 Main app exited. Reading mapped files...\n";
    MappedData a = readMapped(files[0]);
    MappedData b = readMapped(files[1]);

    const MappedData& latest = (a.timestamp >= b.timestamp) ? a : b;
    writeFinal(latest);

    return 0;
}
