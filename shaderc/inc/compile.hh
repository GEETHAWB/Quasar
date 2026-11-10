#pragma once

#include <filesystem>
#include <memory>

namespace qsr::shader {
class Compiler final {
public:
    explicit Compiler(std::filesystem::path shaderDirectory);
    ~Compiler();

    auto compile() -> void;

private:
    class Implementation;
    std::unique_ptr<Implementation> implementation_;
};

}; // namespace qsr::shader
