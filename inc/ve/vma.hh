#pragma once

#define VMA_STATIC_VULKAN_FUNCTIONS 1
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0

#include <memory>
#include <vk_mem_alloc.h>

namespace qsr::ve {

struct VmaAllocatorDeleter final {
    auto operator()(VmaAllocator_T* allocator) const noexcept -> void {
        if (allocator != nullptr)
            vmaDestroyAllocator(allocator);
    }
};

using VmaAllocatorOwner = std::unique_ptr<VmaAllocator_T, VmaAllocatorDeleter>;

struct VmaImageDeleter final {
    VmaAllocator allocator_{};
    VmaAllocation allocation_{};

    auto operator()(VkImage_T* image) const noexcept -> void {
        if (image != nullptr)
            vmaDestroyImage(this->allocator_, image, this->allocation_);
    }
};

using VmaImageOwner = std::unique_ptr<VkImage_T, VmaImageDeleter>;

} // namespace qsr::ve
