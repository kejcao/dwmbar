#include <assert.h>
#include <math.h>
#include <signal.h>
#include <stdint.h>
#include <chrono>
#include <format>
#include <fstream>
#include <functional>
#include <thread>

// to update status bar
#include <X11/Xlib.h>

// to get temperature
#include <sensors/sensors.h>

// to get vram usage
#include <libdrm/amdgpu.h>
#include <libdrm/amdgpu_drm.h>
#include <xf86drm.h>
#include <fcntl.h>

using namespace std::chrono;

// turn int to string with size prefix
std::string human_readable_size(uint64_t n) {
	int i;
	for (i = 0; n > 1024; ++i)
		n /= 1024;

	return std::format("{}{}", n,
		(const char*[]){"", "k", "M", "G", "T"}[i]);
}

namespace modules {
	std::function<std::string()>
		time, celsius, vram, memory, battery;

	void init_vram() {
		drmDevicePtr device;

		assert(drmGetDevices(nullptr, 0) == 1); // make sure we have only one card
		assert(drmGetDevices(&device, 1) >= 0); // get that card

		// try render node first, as it does not require to drop master
		int fd;
		for (int j = DRM_NODE_MAX - 1; j >= 0; --j) {
			if (1 << j & device[0].available_nodes) {
				assert((fd = open(device[0].nodes[j], O_RDWR)) != -1);
				break;
			}
		}

		amdgpu_device_handle dev;
		uint32_t drm_major, drm_minor, out32;
		amdgpu_device_initialize(fd, &drm_major, &drm_minor, &dev);

		struct drm_amdgpu_info_vram_gtt ram;
		amdgpu_query_info(dev, AMDGPU_INFO_VRAM_GTT, sizeof(ram), &ram);

		vram = [dev, ram]() {
			uint64_t vram, gtt;
			amdgpu_query_info(dev, AMDGPU_INFO_VRAM_USAGE, sizeof(uint64_t), &vram);
			amdgpu_query_info(dev, AMDGPU_INFO_GTT_USAGE, sizeof(uint64_t), &gtt);

			return std::format("{}%", std::round((double)vram / ram.vram_size * 100));
		};
	}

	void init_celsius() {
		sensors_init(NULL);
		sensors_chip_name name;
		sensors_parse_chip_name("thinkpad-isa-0000", &name);

		int a = 0, b = 1, c = 0;
		auto chip = sensors_get_detected_chips(&name, &a);
		auto f = sensors_get_features(chip, &b);
		auto s = sensors_get_all_subfeatures(chip, f, &c);

		celsius = [chip, s]() {
			double t;
			sensors_get_value(chip, s->number, &t);
			return std::format("{}", t);
		};
	}

	void init() {
		init_vram();
		init_celsius();

		time = []() {
			return std::format("{:%a %b %e %I:%M:%S %p %Z %Y}",
				zoned_time(current_zone(), floor<seconds>(system_clock::now())));
		};

		memory = []() {
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
		};

		battery = []() {
			auto ifs = std::ifstream("/sys/class/power_supply/BAT0/capacity");
			std::string line;
			std::getline(ifs, line);
			return line;
		};
	}
};

Display *dpy = XOpenDisplay(NULL);

void update() {
	XStoreName(
		dpy, DefaultRootWindow(dpy),
		std::format(" {} {}  +{}°C {}%  {} ",
			modules::vram(),
			modules::memory(),
			modules::celsius(),
			modules::battery(),
			modules::time()).c_str());

	XSync(dpy, False);
	std::this_thread::sleep_for(seconds(1));
}

int main(void) {
	modules::init();

	signal(SIGUSR1, (sighandler_t)update);
	for (;;)
		update();
}
