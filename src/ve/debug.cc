#include "helper.hh"
#include "ve/renderer.hh"

#ifdef DEBUG_MODE
static VKAPI_ATTR VKAPI_CALL auto debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                                                vk::DebugUtilsMessageTypeFlagsEXT,
                                                vk::DebugUtilsMessengerCallbackDataEXT const* data,
                                                [[maybe_unused]] void* userData) noexcept
    -> vk::Bool32 {
    if (data == nullptr || data->pMessage == nullptr)
        return vk::False;

    auto level = ::LogLevel::Debug;

    switch (severity) {
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
            [[fallthrough]];
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
            level = ::LogLevel::Warn;
            break;
        default:
            break;
    }

    try {
        ::output(std::string_view{data->pMessage}, level);
    } catch (...) {
        // Exceptions must not escape through the Vulkan callback
    }

    return vk::False;
}
#endif

namespace qsr::ve {

#ifdef DEBUG_MODE
auto Renderer::createDebugMessenger() -> void {
    auto debugCI = vk::DebugUtilsMessengerCreateInfoEXT{};
    debugCI.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                              vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                              vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                              vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
    debugCI.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                          vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                          vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

    debugCI.pfnUserCallback = &debugCallback;

    debugMessenger_ = vk::raii::DebugUtilsMessengerEXT{instance_, debugCI};
    ::output("Created debug messenger", ::LogLevel::Info);
}
#else
auto Renderer::createDebugMessenger() -> void {
}
#endif

} // namespace qsr::ve
