#pragma once

#include "types.hh"

#include <unordered_map>
#include <vulkan/vulkan_raii.hpp>

namespace qsr::ve {

struct QCommandBuffers final {
    vk::raii::CommandBuffers primary{nullptr};
    vk::raii::CommandBuffers secondary{nullptr};
};

struct QCommandPool final {
    vk::raii::CommandPool pool{nullptr};
    QCommandBuffers buffers{};
    qsr::u32 queueFamilyIndex{};

    auto create(const vk::raii::Device& device,
                qsr::u32 queueIndex,
                qsr::u32 primaryCount,
                qsr::u32 secondaryCount) -> void;
    auto allocatePrimary(const vk::raii::Device& device, qsr::u32 count) -> void;
    auto allocateSecondary(const vk::raii::Device& device, qsr::u32 count) -> void;
    auto reset() -> void;
};

struct QThreadCommandPools final {
    std::unordered_map<qsr::u32, QCommandPool> pools{};

    auto create(const vk::raii::Device& device,
                std::span<const qsr::u32> queueFamilyIndices,
                qsr::u32 primaryount,
                qsr::u32 secondaryCount) -> void;
};

struct QFrameCommandPools final {
    std::vector<QThreadCommandPools> threads{};

    auto create(const vk::raii::Device& device,
                qsr::u32 threadCount,
                std::span<const qsr::u32> queueFamilyIndices,
                qsr::u32 primaryCount,
                qsr::u32 secondaryCount) -> void;
};

struct QCommandContext final {
    std::array<QFrameCommandPools, framesInFlight> frames{};
    qsr::u32 workerThreadCount{};
    qsr::u32 recordingThreadCount{};

    auto create(const vk::raii::Device& device,
                qsr::u32 threadCount,
                std::span<const qsr::u32> queueFamilyIndices,
                qsr::u32 primaryCount,
                qsr::u32 secondaryCount) -> void;
    [[nodiscard]] auto getPool(qsr::u32 frameIndex, qsr::u32 threadIndex, qsr::u32 queueFamilyIndex)
        -> QCommandPool&;
};

} // namespace qsr::ve
