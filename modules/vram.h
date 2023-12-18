#include <assert.h>
#include <math.h>
#include <format>
#include <string>

#include <libdrm/amdgpu.h>
#include <libdrm/amdgpu_drm.h>
#include <xf86drm.h>
#include <fcntl.h>

namespace modules {
    std::string vram() {
        static auto [total, dev] = []() {
            drmDevicePtr device;

            assert(drmGetDevices(nullptr, 0) == 1); // make sure we have only one card
            assert(drmGetDevices(&device, 1) >= 0); // get that card

            // try render node first, as it does not require to drop master
            int fd = -1;
            for (int j = DRM_NODE_MAX - 1; j >= 0; --j) {
                if (1 << j & device[0].available_nodes) {
                    fd = open(device[0].nodes[j], O_RDWR);
                    break;
                }
            }
            assert(fd != -1);

            amdgpu_device_handle dev;
            uint32_t major, minor;
            amdgpu_device_initialize(fd, &major, &minor, &dev);

            struct drm_amdgpu_info_vram_gtt total;
            amdgpu_query_info(dev, AMDGPU_INFO_VRAM_GTT, sizeof(total), &total);

            return std::pair{total, dev};
        }();


        uint64_t vram, gtt;
        amdgpu_query_info(dev, AMDGPU_INFO_VRAM_USAGE, sizeof(uint64_t), &vram);
        amdgpu_query_info(dev, AMDGPU_INFO_GTT_USAGE, sizeof(uint64_t), &gtt);

        return std::format("{}%", std::round((double)vram / total.vram_size * 100));
    }
}