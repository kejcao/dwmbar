#include <format>
#include <string>

#include <sensors/sensors.h>

namespace modules {
	std::string celsius() {
		static auto [chip, s] = []() {
			sensors_init(NULL);
			sensors_chip_name name;
			sensors_parse_chip_name("thinkpad-isa-0000", &name);

			int a = 0, b = 1, c = 0;
			auto chip = sensors_get_detected_chips(&name, &a);
			auto f = sensors_get_features(chip, &b);
			auto s = sensors_get_all_subfeatures(chip, f, &c);
			return std::pair{chip, s};
		}();

		double t;
		sensors_get_value(chip, s->number, &t);
		return std::format("{}", t);
	}
}