#pragma once

#include <SDL.h>
#include <string_view>

namespace qsr::sdl {

class Window final {
  public:
    Window(std::string_view title, int width, int height);

    ~Window() noexcept;

    // Returns a non-owning pointer for the renderer
    [[nodiscard]] auto nativeHandle() const noexcept -> SDL_Window* { return window_; }

  private:
    SDL_Window* window_{nullptr};
    bool videoSubsystemInitialized_{false};
};

}; // namespace qsr::sdl
