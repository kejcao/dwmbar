#include <string>
#include <fstream>
#include <format>

namespace modules {
    std::string memory() {
        long totalram, totalswap, freeram, freeswap;

        std::ifstream file("/proc/meminfo");
        std::string line;

        while (std::getline(file, line)) {
            if (sscanf(line.c_str(), "MemTotal: %ld kB", &totalram) == 1 ||
                sscanf(line.c_str(), "MemAvailable: %ld kB", &freeram) == 1 ||
                sscanf(line.c_str(), "SwapTotal: %ld kB", &totalswap) == 1)
                continue;

            if (sscanf(line.c_str(), "SwapFree: %ld kB", &freeswap) == 1)
                break;
        }

        return std::format("{:.0f}% {:.0f}%",
            (double)(totalram - freeram) / totalram * 100,
            (double)(totalswap - freeswap) / totalswap * 100);
    }
}