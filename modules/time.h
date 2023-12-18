#include <chrono>
#include <format>
#include <string>

namespace modules {
	std::string time() {
		using namespace std::chrono;

		return std::format("{:%a %b %e %I:%M:%S %p %Z %Y}",
			zoned_time(current_zone(), floor<seconds>(system_clock::now())));
	}
}