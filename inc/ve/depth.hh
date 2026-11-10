#pragma once

#include "vma.hh"

#include <vulkan/vulkan_raii.hpp>

namespace qsr::ve {

struct QDepthAttachment final {
    VmaImageOwner image{nullptr, VmaImageDeleter{}};
    vk::raii::ImageView view{nullptr};

    vk::Format format{vk::Format::eUndefined};
};

}; // namespace qsr::ve
