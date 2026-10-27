#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace qsr {

class Engine final {
  public:
    Engine() { createInstance(); }

  private:
    auto createInstance() -> void;

    vk::raii::Context context_{};
    vk::raii::Instance instance_{nullptr};
};

}; // namespace qsr
