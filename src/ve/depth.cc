#include "helper.hh"
#include "ve/renderer.hh"
#include "ve/vma.hh"
#include "vulkan/vulkan.hpp"

#include <array>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace qsr::ve {

auto Renderer::createDepthAttachment() -> void {
    if (this->swapchain_.swapchainExtent_.width == 0 ||
        this->swapchain_.swapchainExtent_.height == 0)
        throw std::invalid_argument{"Depth attachment extent cannot be zero"};

    constexpr std::array depthFormats{
        vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint};

    auto depthFormat = vk::Format::eUndefined;

    for (const auto format : depthFormats) {
        const auto properties = this->physicalDevice_.getFormatProperties(format);

        if (properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
            depthFormat = format;
            break;
        }
    }

    if (depthFormat == vk::Format::eUndefined)
        throw std::runtime_error{"No supported depth attachment format found"};

    vk::ImageCreateInfo imageCI{};
    imageCI.setImageType(vk::ImageType::e2D)
        .setFormat(depthFormat)
        .setExtent(
            {this->swapchain_.swapchainExtent_.width, this->swapchain_.swapchainExtent_.height, 1})
        .setMipLevels(1)
        .setArrayLayers(1)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setTiling(vk::ImageTiling::eOptimal)
        .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
        .setSharingMode(vk::SharingMode::eExclusive)
        .setInitialLayout(vk::ImageLayout::eUndefined);

    const auto& nativeImageCI = static_cast<const VkImageCreateInfo&>(imageCI);

    // Ugly but required syntax
    const VmaAllocationCreateInfo allocationCI = [] {
        VmaAllocationCreateInfo info{};
        info.usage = VMA_MEMORY_USAGE_AUTO;
        info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        return info;
    }();

    VkImage rawImage = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;

    // Fuck this function call
    const auto result = vmaCreateImage(
        this->allocator_.get(), &nativeImageCI, &allocationCI, &rawImage, &allocation, nullptr);

    if (result != VK_SUCCESS)
        throw std::runtime_error{std::format("Quasar: vmaCreateImage failed with VkResult {}",
                                             static_cast<qsr::i32>(result))};

    auto attachment = QDepthAttachment{};
    attachment.format = depthFormat;
    attachment.image = VmaImageOwner{rawImage, VmaImageDeleter{this->allocator_.get(), allocation}};

    auto aspectMask = vk::ImageAspectFlags{vk::ImageAspectFlagBits::eDepth};
    if (depthFormat == vk::Format::eD32SfloatS8Uint || depthFormat == vk::Format::eD24UnormS8Uint) {
        aspectMask |= vk::ImageAspectFlagBits::eStencil;
    }

    ::output("Depth attachment created", ::LogLevel::Info);
}

}; // namespace qsr::ve
