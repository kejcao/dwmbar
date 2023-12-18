#include <signal.h>
#include <chrono>
#include <format>
#include <thread>

#include <X11/Xlib.h>

// // util function for header files
// std::string human_readable_size(uint64_t n) {
// 	int i;
// 	for (i = 0; n > 1024; ++i)
// 		n /= 1024;

// 	return std::format("{}{}", n,
// 		(const char*[]){"", "k", "M", "G", "T"}[i]);
// }

#include "modules/battery.h"
#include "modules/memory.h"
#include "modules/vram.h"
#include "modules/celsius.h"
#include "modules/time.h"

Display *dpy = XOpenDisplay(NULL);

void update() {
	using std::chrono::seconds;

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
	signal(SIGUSR1, (sighandler_t)update);
	for (;;)
		update();
}
