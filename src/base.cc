#include "engine.hh"
#include "types.hh"
#include <chrono>
#include <print>
#include <vulkan/vulkan_core.h>

namespace qsr {

auto Engine::createInstance() -> void {
#ifdef DEBUG_MODE
    const auto start = std::chrono::steady_clock::now();
#endif

    constexpr qsr::u32 optimalApiVersion = VK_API_VERSION_1_4;
    const qsr::u32 loaderApiVersion = context_.enumerateInstanceVersion();

    if (loaderApiVersion < optimalApiVersion) {
        std::println("[Quasar] Warning: Vulkan 1.4 is unavailable. Some features may not work. \n "
                     "Loader suports: {}.{}",
                     VK_API_VERSION_MAJOR(loaderApiVersion),
                     VK_API_VERSION_MINOR(loaderApiVersion));
    }

    const qsr::u32 apiVersion = std::min(loaderApiVersion, optimalApiVersion);

    vk::ApplicationInfo appInfo{};
    appInfo.pApplicationName = "Quasar";
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    appInfo.pEngineName = "Quasar Engine";
    appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    appInfo.apiVersion = apiVersion;

    vk::InstanceCreateInfo instanceCI{};
    instanceCI.pApplicationInfo = &appInfo;

    // RAII objects automatically throw exceptions
    instance_ = vk::raii::Instance{context_, instanceCI};

#ifdef DEBUG_MODE
    const auto end = std::chrono::steady_clock::now();

    const auto elapsed = std::chrono::duration<double, std::milli>(end - start);

    std::println("[Quasar] Info: Instance creation successful in {} ms", elapsed.count());
#endif
}

}; // namespace qsr
