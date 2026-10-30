#pragma once

#include "ve/renderer.hh"
#include "window.hh"

namespace qsr {

class Engine final {
public:
    Engine() {
    }

private:
    sdl::Window window_{"Quasar", 1280, 720};
    ve::Renderer renderer_{window_};
};

}; // namespace qsr
