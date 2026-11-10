#define VMA_IMPLEMENTATION

#include "ve/vma.hh"

#include "helper.hh"
#include "ve/renderer.hh"

#include <stdexcept>

namespace qsr::ve {

auto Renderer::createAllocator() -> void {
    if (this->allocator_)
        throw std::logic_error{"Quasar: VMA allocator has already been initialized"};

    VmaAllocatorCreateInfo vmaCI{};
    vmaCI.instance = static_cast<VkInstance>(*this->instance_);
    vmaCI.physicalDevice = static_cast<VkPhysicalDevice>(*this->physicalDevice_);
    vmaCI.device = static_cast<VkDevice>(*this->device_);
    vmaCI.vulkanApiVersion = this->apiVersion_;

    VmaAllocator rawAllocator{};

    if (const auto result = vmaCreateAllocator(&vmaCI, &rawAllocator); result != VK_SUCCESS) {
        throw std::runtime_error{std::format("Quasar: Failed to create VMA allocator {}",
                                             static_cast<qsr::i32>(result))};
    }

    this->allocator_.reset(rawAllocator);

    ::output("VMA initialized", ::LogLevel::Info);
}

}; // namespace qsr::ve
