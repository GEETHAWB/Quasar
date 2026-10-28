#include "engine.hh"
#include <exception>
#include <print>

auto main() -> int {
    try {
        qsr::Engine engine;
    } catch (const std::exception& e) {
        std::println("Initialization Failed: {}", e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
