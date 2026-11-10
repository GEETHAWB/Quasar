#include "window.hh"

#include "helper.hh"
#include "types.hh"

#include <SDL.h>
#include <SDL_video.h>
#include <stdexcept>
#include <string>

namespace qsr::sdl {

Window::Window(std::string_view title, int width, int height) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("Window dimensions must be positive");
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
    }

    this->videoSubsystemInitialized_ = true;

    u32 flags = SDL_WINDOW_VULKAN;

    // Make sure the window's resizable and enable high dpi
    flags |= SDL_WINDOW_RESIZABLE;
    flags |= SDL_WINDOW_ALLOW_HIGHDPI;

    const std::string windowTitle{title};

    window_ = SDL_CreateWindow(
        windowTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);

    ::output("SDL Window Created", ::LogLevel::Success);

    if (this->window_ == nullptr) {
        const std::string error = SDL_GetError();

        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        this->videoSubsystemInitialized_ = false;

        throw std::runtime_error("SDL_CreateWindow failed: " + error);
    }
}

Window::~Window() noexcept {
    if (this->window_ != nullptr)
        SDL_DestroyWindow(this->window_);

    if (this->videoSubsystemInitialized_)
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

} // namespace qsr::sdl
