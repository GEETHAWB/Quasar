#include "ve/renderer.hh"

using namespace qsr::ve;

auto Renderer::init() -> void {
    createInstance();
    createDebugMessenger();
    createSurface();
    createPhysicalDevice();
    createQueueFamilies();
    createLogicalDevice();
}
