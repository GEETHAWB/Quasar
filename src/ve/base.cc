#include "helper.hh"
#include "types.hh"
#include "ve/renderer.hh"

#include <SDL_error.h>
#include <SDL_vulkan.h>
#include <algorithm>
#include <chrono>
#include <exception>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

using namespace qsr::ve;

namespace {

// Check through available validation layers for the desired one
[[nodiscard]] auto hasLayer(const std::vector<vk::LayerProperties>& availableLayers,
                            std::string_view name) -> bool {
    return std::any_of(
        availableLayers.begin(), availableLayers.end(),
        [name](const vk::LayerProperties& layer) { return name == layer.layerName; });
}

// Check through available extensions for the desired one
[[nodiscard]] auto hasExtension(const std::vector<vk::ExtensionProperties>& availableExtensions,
                                std::string_view name) -> bool {
    return std::any_of(availableExtensions.begin(), availableExtensions.end(),
                       [name](const vk::ExtensionProperties& extension) {
                           return name == extension.extensionName;
                       });
}

// What tf does this do?
auto appendUnique(std::vector<const char*>& extensions, const char* name) -> void {
    const bool alreadyPresent =
        std::any_of(extensions.begin(), extensions.end(),
                    [name](const char* existing) { return std::string_view{existing} == name; });

    if (!alreadyPresent)
        extensions.push_back(name);
}

}; // namespace

auto Renderer::createInstance() -> void {

#ifdef DEBUG_MODE

    auto start = std::chrono::steady_clock::now();

    constexpr const char* validationLayerName = "VK_LAYER_KHRONOS_validation";

    const auto availableLayers = context_.enumerateInstanceLayerProperties();

    std::vector<const char*> enabledLayers;

    if (!(::hasLayer(availableLayers, validationLayerName))) {
        throw std::runtime_error("Quasar: Required validation layer is unavailable. \n"
                                 "Is Vulkan installed properly?: " +
                                 std::string{validationLayerName});
    }
    enabledLayers.push_back(validationLayerName);
#endif

    // Due to the fact that SDL is a C library, this
    // function needs to be called twice for the count and then for the actual extensions

    qsr::u32 extensionCount = 0;

    if (SDL_Vulkan_GetInstanceExtensions(window_.nativeHandle(), &extensionCount, nullptr) !=
        SDL_TRUE) {
        throw std::runtime_error(std::string{"Quasar: SDL extension query failed: "} +
                                 SDL_GetError());
    }

    std::vector<const char*> sdlExtensions(extensionCount);

    if (SDL_Vulkan_GetInstanceExtensions(window_.nativeHandle(), &extensionCount,
                                         sdlExtensions.data()) != SDL_TRUE) {
        throw std::runtime_error(std::string{"Quasar: SDL extension retrieval failed: "} +
                                 SDL_GetError());
    }

    const auto availableExtensions = context_.enumerateInstanceExtensionProperties();

    std::vector<const char*> enabledExtensions;

    for (const char* extension : sdlExtensions) {
        if (!(::hasExtension(availableExtensions, extension))) {
            throw std::runtime_error("Quasar: Required Vulkan instance extension is unavailable: " +
                                     std::string{extension});
        }

        ::appendUnique(enabledExtensions, extension);
    }

#ifdef DEBUG_MODE
    if (!(::hasExtension(availableExtensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))) {
        throw std::runtime_error("Quasar: VK_EXT_debug_utils is unavailable");
    }

    ::appendUnique(enabledExtensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    const qsr::u32 availableVersion = context_.enumerateInstanceVersion();

    if (availableVersion < VK_API_VERSION_1_4) {
        const auto msg = std::format("Vulkan Version {}.{} detected. Vulkan 1.4 is preferred."
                                     "Some features may not work properly.",
                                     VK_API_VERSION_MAJOR(availableVersion),
                                     VK_API_VERSION_MINOR(availableVersion));
        ::output(msg, ::LogLevel::Warn);
    }

    // Actually making the damn instance
    vk::ApplicationInfo appInfo{};
    appInfo.pApplicationName = "Quasar";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "QuasarEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_4;

    vk::InstanceCreateInfo instanceCI{};
    instanceCI.pApplicationInfo = &appInfo;
    instanceCI.enabledLayerCount = static_cast<qsr::u32>(enabledLayers.size());
    instanceCI.ppEnabledLayerNames = enabledLayers.data();
    instanceCI.enabledExtensionCount = static_cast<qsr::u32>(enabledExtensions.size());
    instanceCI.ppEnabledExtensionNames = enabledExtensions.data();

    // RAII functions automatically throw exceptions if things go wrong
    instance_ = vk::raii::Instance(context_, instanceCI);

#ifdef DEBUG_MODE
    auto end = std::chrono::steady_clock::now();
    const auto msg =
        std::format("vkInstance created in {} ms", ::elapsed(start, end, ::Time::Millis));
    ::output(msg, ::LogLevel::Info);
#endif
}

auto Renderer::createSurface() -> void {
    const auto nativeWindow = window_.nativeHandle();

    if (nativeWindow == nullptr)
        throw std::runtime_error{"Quasar: SDL Window Handle is null"};

    VkSurfaceKHR rawSurface = VK_NULL_HANDLE;

    if (SDL_Vulkan_CreateSurface(nativeWindow, *instance_, &rawSurface) != SDL_TRUE) {
        const char* const error = SDL_GetError();

        throw std::runtime_error("Quasar: SDL Vulkan surface creation failed");
    }

    surface_ = vk::raii::SurfaceKHR{instance_, rawSurface};

#ifdef DEBUG_MODE
    ::output("Vulkan surface created successfully", ::LogLevel::Info);
#endif
}

auto Renderer::createPhysicalDevice() -> void {
    try {
        auto devices = instance_.enumeratePhysicalDevices();

        if (devices.empty())
            throw std::runtime_error{"Quasar: No Vulkan-Compatible physical devices found."};

        // Simple device selection for now
        // TODO: Once logical device is done, implement more complex scoring
        physicalDevice_ = std::move(devices.front());

#ifdef DEBUG_MODE
        const auto props = physicalDevice_.getProperties();
        const auto msg =
            std::format("Selected physical device: {}", std::string_view{props.deviceName});
        ::output(msg, ::LogLevel::Info);
#endif
    } catch (const vk::SystemError& e) {
        std::throw_with_nested(std::runtime_error{
            std::format("Quasar: Physical device creation failed: {}", e.what())});
    }
}

auto Renderer::createQueueFamilies() -> void {
    try {
        const auto families = physicalDevice_.getQueueFamilyProperties();
        QueueFamilyIndices indices{};

        for (qsr::usize i = 0; i < families.size(); ++i) {
            const auto& family = families[i];
            const auto index = static_cast<qsr::u32>(i);

            if (family.queueCount == 0)
                continue;

            const bool supportsGraphics =
                (family.queueFlags & vk::QueueFlagBits::eGraphics) != vk::QueueFlags{};

            const bool supportsPresentation =
                physicalDevice_.getSurfaceSupportKHR(index, *surface_);

            // Prefer a family with both available
            if (supportsGraphics && supportsPresentation) {
                indices.graphics = index;
                indices.presentation = index;
                break;
            }

            // Otherwise, find first suitable family for each
            if (supportsGraphics && !indices.graphics)
                indices.graphics = index;

            if (supportsPresentation && !indices.presentation)
                indices.presentation = index;
        }
        if (!indices.isComplete())
            throw std::runtime_error{
                "Quasar: No suitable graphics and presentation queue families were found."};

#ifdef DEBUG_MODE
        if (*indices.graphics == *indices.presentation) {
            ::output(std::format("Using queue family {} for graphics and presentation",
                                 *indices.graphics),
                     ::LogLevel::Info);
        } else {
            ::output(std::format("Graphics queue family: {}", *indices.graphics), ::LogLevel::Info);
            ::output(std::format("Presentation queue family: {}", *indices.presentation),
                     ::LogLevel::Info);
        }
#endif
    } catch (const vk::SystemError& e) {
        std::throw_with_nested(
            std::runtime_error{std::format("Quasar: Queue family creation failed: {}", e.what())});
    }
}
