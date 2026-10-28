#pragma once

#include "../window.hh"
#include "types.hh"

#include <vulkan/vulkan_raii.hpp>

namespace qsr::ve {

struct QueueFamilyIndices final {
    std::optional<qsr::u32> graphics;
    std::optional<qsr::u32> presentation;

    [[nodiscard]] inline constexpr auto isComplete() const noexcept -> bool {
        return graphics.has_value() && presentation.has_value();
    }
};

class Renderer final {
  public:
    explicit Renderer(sdl::Window& window) : window_{window} { init(); };
    ~Renderer() = default;

  private:
    auto init() -> void;
    auto createInstance() -> void;
    auto createPhysicalDevice() -> void;
    auto createSurface() -> void;
    auto createQueueFamilies() -> void;

    const sdl::Window& window_;
    vk::raii::Context context_;
    vk::raii::Instance instance_{nullptr};
    vk::raii::SurfaceKHR surface_{nullptr};
    vk::raii::PhysicalDevice physicalDevice_{nullptr};

    QueueFamilyIndices queueFamilyIndices_{};
};

}; // namespace qsr::ve
