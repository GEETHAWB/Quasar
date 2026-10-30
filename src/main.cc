#include "engine.hh"
#include "helper.hh"

#include <exception>

auto main() -> int {
    try {
        qsr::Engine engine;
    } catch (const std::exception& e) {
        ::output(std::format("Initialization Failed: {}", e.what()), ::LogLevel::Error);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
