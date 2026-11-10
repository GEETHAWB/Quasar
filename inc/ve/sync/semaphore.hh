#pragma once

#include "types.hh"

#include <vector>
#include <vulkan/vulkan_raii.hpp>

namespace qsr::ve {
struct QQueueTimeline final {
    vk::raii::Semaphore semaphore{nullptr};
    qsr::u64 nextValue{1};
    qsr::u64 lastSubmittedValue{0};
};

struct QFrameSemaphores final {
    vk::raii::Semaphore imageAvailable{nullptr};
};

struct QSemaphoreContext final {
    QQueueTimeline graphics{};
    std::array<QFrameSemaphores, framesInFlight> frames{};
    std::vector<vk::raii::Semaphore> presentReady{};
    bool initialized_{false};

    auto create(const vk::raii::Device& device, qsr::u32 imageCount) -> void;
};

} // namespace qsr::ve
