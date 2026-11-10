#pragma once

#include "types.hh"

#include <optional>

namespace qsr::ve {

struct QueueFamilyIndices final {
    std::optional<qsr::u32> graphics{};
    std::optional<qsr::u32> present{};
    std::optional<qsr::u32> compute{};
    std::optional<qsr::u32> transfer{};

    [[nodiscard]] inline constexpr auto isComplete() const noexcept -> bool {
        return graphics.has_value() && compute.has_value() && graphics.has_value();
    }
};

}; // namespace qsr::ve
