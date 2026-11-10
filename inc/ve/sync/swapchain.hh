#pragma once

#include "window.hh"

#include <SDL_vulkan.h>
#include <vulkan/vulkan_raii.hpp>

namespace qsr::ve {

class QSwapchain final {
public:
    vk::raii::SwapchainKHR swapchain_{nullptr};
    std::vector<vk::Image> swapchainImages_{};
    vk::SurfaceFormatKHR swapchainSurfaceFormat_{};
    vk::Extent2D swapchainExtent_{};
    vk::PresentModeKHR swapchainPresentMode_{};

private:
    [[nodiscard]] auto chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats) const
        -> vk::SurfaceFormatKHR;
    [[nodiscard]] auto choosePresentMode(const std::vector<vk::PresentModeKHR>& modes) const
        -> vk::PresentModeKHR;
    [[nodiscard]] auto chooseExtent(const vk::SurfaceCapabilitiesKHR& capabilities,
                                    sdl::Window& window) const -> vk::Extent2D;

    [[nodiscard]] auto chooseCompositeAlpha(const vk::SurfaceCapabilitiesKHR& capabilities) const
        -> vk::CompositeAlphaFlagBitsKHR;
};

}; // namespace qsr::ve
