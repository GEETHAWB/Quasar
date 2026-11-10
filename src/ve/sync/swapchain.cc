#include "ve/renderer.hh"
#include "vulkan/vulkan.hpp"

#include <SDL_vulkan.h>
#include <stdexcept>

namespace {

[[nodiscard]] auto chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats)
    -> vk::SurfaceFormatKHR {
    constexpr vk::SurfaceFormatKHR preferred{vk::Format::eB8G8R8A8Srgb,
                                             vk::ColorSpaceKHR::eSrgbNonlinear};

    // If the vector is invalid return the preferred and pray it works
    if (formats.size() == 1 && formats.front().format == vk::Format::eUndefined)
        return preferred;

    const auto best = std::ranges::find(formats, preferred);

    return best != formats.end() ? *best : formats.front();
}

[[nodiscard]] auto choosePresentMode(const std::vector<vk::PresentModeKHR>& modes)
    -> vk::PresentModeKHR {
    constexpr auto preferred = vk::PresentModeKHR::eMailbox;

    // FIFO is guaranteed so if we can't find mailbox we fall back to it
    return std::ranges::find(modes, preferred) != modes.end() ? preferred
                                                              : vk::PresentModeKHR::eFifo;
}

[[nodiscard]] auto chooseExtent(const vk::SurfaceCapabilitiesKHR& capabilities,
                                const qsr::sdl::Window& window) -> vk::Extent2D {
    if (capabilities.currentExtent.width != std::numeric_limits<qsr::u32>::max())
        return capabilities.currentExtent;

    int width, height = 0;

    SDL_Vulkan_GetDrawableSize(window.nativeHandle(), &width, &height);

    if (width <= 0 || height <= 0)
        throw std::runtime_error{"Quasar: Invalid Vulkan drawable size"};

    return {std::clamp(static_cast<qsr::u32>(width),
                       capabilities.minImageExtent.width,
                       capabilities.maxImageExtent.width),
            std::clamp(static_cast<qsr::u32>(height),
                       capabilities.minImageExtent.height,
                       capabilities.maxImageExtent.height)};
}

[[nodiscard]] auto chooseImageCount(const vk::SurfaceCapabilitiesKHR& capabilities) -> qsr::u32 {
    auto count = capabilities.minImageCount + 1;

    if (capabilities.maxImageCount != 0)
        count = std::min(count, capabilities.maxImageCount);

    return count;
}

[[nodiscard]] auto chooseCompositeAlpha(const vk::SurfaceCapabilitiesKHR& capabilities)
    -> vk::CompositeAlphaFlagBitsKHR {
    constexpr std::array candidates{
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
        vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
    };

    for (const auto candidate : candidates) {
        if (capabilities.supportedCompositeAlpha & candidate)
            return candidate;
    }

    throw std::runtime_error{"Quasar: No supported composite alpha mode"};
}

}; // namespace

namespace qsr::ve {

auto Renderer::createSwapchain() -> void {
    // Queue families have already been checked at this point,
    // no further validation is necessary.
    const auto capabilities = this->physicalDevice_.getSurfaceCapabilitiesKHR(this->surface_);
    const auto formats = this->physicalDevice_.getSurfaceFormatsKHR(this->surface_);
    const auto presentModes = this->physicalDevice_.getSurfacePresentModesKHR(this->surface_);

    if (formats.empty())
        throw std::runtime_error{"Quasar: No surface formats available"};

    if (presentModes.empty())
        throw std::runtime_error{"Quasar: No presentation modes available"};

    const auto surfaceFormat = ::chooseSurfaceFormat(formats);
    const auto imageCount = ::chooseImageCount(capabilities);
    const auto extent = ::chooseExtent(capabilities, this->window_);

    const auto graphicsFamily = this->queueFamilyIndices_.graphics.value();
    const auto presentFamily = this->queueFamilyIndices_.compute.value();

    const std::array queueFamilies{graphicsFamily, presentFamily};

    // Initial swapchain create info
    vk::SwapchainCreateInfoKHR swapchainCI{{},
                                           this->surface_,
                                           imageCount,
                                           surfaceFormat.format,
                                           surfaceFormat.colorSpace,
                                           extent,
                                           1,
                                           vk::ImageUsageFlagBits::eColorAttachment};

    if (graphicsFamily != presentFamily) {
        swapchainCI.imageSharingMode = vk::SharingMode::eConcurrent;
        swapchainCI.queueFamilyIndexCount = static_cast<qsr::u32>(queueFamilies.size());
        swapchainCI.pQueueFamilyIndices = queueFamilies.data();
    } else {
        swapchainCI.imageSharingMode = vk::SharingMode::eExclusive;
    }

    swapchainCI.preTransform = capabilities.currentTransform;
    swapchainCI.compositeAlpha = ::chooseCompositeAlpha(capabilities);
    swapchainCI.presentMode = ::choosePresentMode(presentModes);
    swapchainCI.clipped = vk::True;

    // Upon initialization, the original swapchain is constructed as a {nullptr} so this should
    // always work.
    swapchainCI.oldSwapchain = this->swapchain_.swapchain_;

    vk::raii::SwapchainKHR newSwapchain{this->device_, swapchainCI};

    auto newImages = newSwapchain.getImages();
    swapchain_.swapchain_ = std::move(newSwapchain);
    swapchain_.swapchainImages_ = std::move(newImages);
    swapchain_.swapchainSurfaceFormat_ = ::chooseSurfaceFormat(formats);
    swapchain_.swapchainExtent_ = extent;
    swapchain_.swapchainPresentMode_ = ::choosePresentMode(presentModes);
}

}; // namespace qsr::ve
