#include "ve/renderer.hh"

using namespace qsr::ve;

auto Renderer::init() -> void {
    createInstance();
    createSurface();
    createPhysicalDevice();
    createQueueFamilies();
    // TODO: Create logical device & debug messenger
}
