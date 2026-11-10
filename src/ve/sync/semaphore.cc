#include "ve/sync/semaphore.hh"

#include "helper.hh"
#include "ve/renderer.hh"

#include <stdexcept>

namespace qsr::ve {

auto QSemaphoreContext::create(const vk::raii::Device& device, qsr::u32 imageCount) -> void {
    if (this->initialized_)
        throw std::logic_error{"Quasar: Semaphore context has already been initialized"};
    if (imageCount == 0)
        throw std::invalid_argument{"Quasar: Cannot create semaphores without swapchain images"};

    // Timeline semaphore
    vk::SemaphoreTypeCreateInfo timelineInfo{};
    timelineInfo.semaphoreType = vk::SemaphoreType::eTimeline;
    timelineInfo.initialValue = 0;

    vk::SemaphoreCreateInfo timelineCI{};
    timelineCI.pNext = &timelineInfo;
    this->graphics.semaphore = vk::raii::Semaphore{device, timelineCI};

    // Frame acquisition semaphores
    const vk::SemaphoreCreateInfo binaryCI{};
    for (auto& frame : this->frames) {
        frame.imageAvailable = vk::raii::Semaphore{device, binaryCI};
    }

    // Presentation semaphores
    this->presentReady.reserve(imageCount);
    for (qsr::u32 i = 0; i < imageCount; ++i) {
        this->presentReady.emplace_back(device, binaryCI);
    }

    this->initialized_ = true;
}

auto Renderer::createSemaphoreContext() -> void {
    const auto imageCount = static_cast<qsr::u32>(this->swapchain_.swapchainImages_.size());

    QSemaphoreContext context{};
    context.create(this->device_, imageCount);
    this->semaphoreContext_ = std::move(context);

    ::output("Created semaphores", ::LogLevel::Info);
}
}; // namespace qsr::ve
