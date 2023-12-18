#include <fstream>
#include <string>

namespace modules {
    std::string battery() {
        auto ifs = std::ifstream("/sys/class/power_supply/BAT0/capacity");
        std::string line;
        std::getline(ifs, line);
        return line;
    }
}