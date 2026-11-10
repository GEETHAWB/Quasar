#include "ve/sync/command.hh"

#include "helper.hh"
#include "ve/renderer.hh"
#include "vulkan/vulkan.hpp"

#include <thread>

namespace qsr::ve {

auto QCommandPool::create(const vk::raii::Device& device,
                          qsr::u32 queueIndex,
                          qsr::u32 primaryCount,
                          qsr::u32 secondaryCount) -> void {
    vk::CommandPoolCreateInfo commandPoolCI{};
    commandPoolCI.flags = vk::CommandPoolCreateFlagBits::eTransient;
    commandPoolCI.queueFamilyIndex = queueFamilyIndex;

    this->pool = vk::raii::CommandPool{device, commandPoolCI};

    this->queueFamilyIndex = queueIndex;

    if (primaryCount > 0) {
        vk::CommandBufferAllocateInfo allocateCI{};
        allocateCI.commandPool = *this->pool;
        allocateCI.level = vk::CommandBufferLevel::ePrimary;
        allocateCI.commandBufferCount = primaryCount;

        this->buffers.primary = vk::raii::CommandBuffers{device, allocateCI};
    }

    if (secondaryCount > 0) {
        vk::CommandBufferAllocateInfo allocateCI{};
        allocateCI.commandPool = *this->pool;
        allocateCI.level = vk::CommandBufferLevel::eSecondary;
        allocateCI.commandBufferCount = secondaryCount;

        this->buffers.secondary = vk::raii::CommandBuffers{device, allocateCI};
    }
}

auto QCommandPool::reset() -> void {
    this->pool.reset();
}

auto QThreadCommandPools::create(const vk::raii::Device& device,
                                 std::span<const qsr::u32> queueFamilyIndices,
                                 qsr::u32 primaryCount,
                                 qsr::u32 secondaryCount) -> void {
    for (const auto familyIndex : queueFamilyIndices) {
        auto [iter, inserted] = this->pools.try_emplace(familyIndex);
        if (inserted)
            iter->second.create(device, familyIndex, primaryCount, secondaryCount);
    }
}

auto QFrameCommandPools::create(const vk::raii::Device& device,
                                qsr::u32 threadCount,
                                std::span<const qsr::u32> queueFamilyIndices,
                                qsr::u32 primaryCount,
                                qsr::u32 secondaryCount) -> void {
    this->threads.resize(threadCount);

    for (auto& thread : this->threads)
        thread.create(device, queueFamilyIndices, primaryCount, secondaryCount);
}

auto QCommandContext::create(const vk::raii::Device& device,
                             qsr::u32 threadCount,
                             std::span<const qsr::u32> queueFamilyIndices,
                             qsr::u32 primaryCount,
                             qsr::u32 secondaryCount) -> void {
    for (auto& frame : this->frames) {
        frame.create(device, threadCount, queueFamilyIndices, primaryCount, secondaryCount);
    }
}

[[nodiscard]] auto QCommandContext::getPool(qsr::u32 frameIndex,
                                            qsr::u32 threadIndex,
                                            qsr::u32 queueFamilyIndex) -> QCommandPool& {
    return this->frames.at(frameIndex).threads.at(threadIndex).pools.at(queueFamilyIndex);
}

auto Renderer::createCommandObjects() -> void {
    QCommandContext context{};

    // TODO: Less shit implementation
    const auto workerCount = std::thread::hardware_concurrency();

    context.workerThreadCount = workerCount;
    context.recordingThreadCount = context.workerThreadCount + 1;

    std::vector<qsr::u32> queueFamilyIndices{};
    const auto addQueueFamily = [&](qsr::u32 index) -> void {
        if (std::ranges::find(queueFamilyIndices, index) == queueFamilyIndices.end()) {
            queueFamilyIndices.push_back(index);
        }
    };

    addQueueFamily(this->queueFamilyIndices_.graphics.value());
    addQueueFamily(this->queueFamilyIndices_.compute.value());
    if (this->queueFamilyIndices_.transfer)
        addQueueFamily(this->queueFamilyIndices_.transfer.value());

    this->commandContext_.create(this->device_, workerCount, queueFamilyIndices, 1, 1);

    ::output("Created command objects", ::LogLevel::Info);
}

}; // namespace qsr::ve
