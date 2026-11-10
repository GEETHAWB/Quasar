#pragma once

#include "depth.hh"
#include "queue.hh"
#include "sync/swapchain.hh"
#include "ve/sync/command.hh"
#include "ve/sync/semaphore.hh"
#include "vma.hh"
#include "window.hh"

#include <vulkan/vulkan_raii.hpp>

namespace qsr::ve {
class Renderer final {
public:
    explicit Renderer(sdl::Window& window) : window_{window} {
        init();
    };
    ~Renderer() = default;

private:
    auto init() -> void;
    auto createInstance() -> void;
    auto createPhysicalDevice() -> void;
    auto createSurface() -> void;
    auto createQueueFamilies() -> void;
    auto createDebugMessenger() -> void;
    auto createLogicalDevice() -> void;
    auto createSwapchain() -> void;
    auto createAllocator() -> void;
    auto createDepthAttachment() -> void;
    auto createSemaphoreContext() -> void;
    auto createCommandObjects() -> void;

    qsr::u32 apiVersion_;
    const sdl::Window& window_;
    vk::raii::Context context_;
    vk::raii::Instance instance_{nullptr};
    vk::raii::DebugUtilsMessengerEXT debugMessenger_{nullptr};
    vk::raii::SurfaceKHR surface_{nullptr};
    vk::raii::PhysicalDevice physicalDevice_{nullptr};
    vk::raii::Device device_{nullptr};
    QSwapchain swapchain_{};
    VmaAllocatorOwner allocator_{};
    vk::raii::Queue graphicsQueue_{nullptr};
    vk::raii::Queue computeQueue_{nullptr};
    QDepthAttachment depthAttachment_{};
    QSemaphoreContext semaphoreContext_{};
    QCommandContext commandContext_{};
    QueueFamilyIndices queueFamilyIndices_{};
    qsr::u32 currentFrame_{0};
};

}; // namespace qsr::ve
