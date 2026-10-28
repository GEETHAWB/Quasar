#pragma once

#include "types.hh"
#include <chrono>
#include <iostream>

namespace {

enum class Time {
    Secs,
    Millis,
    Micros,
    Nanos,
};

template <typename Clock, typename StartDuration, typename EndDuration>
[[nodiscard]] auto inline elapsed(const std::chrono::time_point<Clock, StartDuration>& start,
                                  const std::chrono::time_point<Clock, EndDuration>& end,
                                  ::Time unit) -> qsr::f32 {
    const auto duration = end - start;

    switch (unit) {
    case ::Time::Secs:
        return std::chrono::duration<qsr::f32>(duration).count();
    case ::Time::Millis:
        return std::chrono::duration<qsr::f32, std::milli>(duration).count();
    case ::Time::Micros:
        return std::chrono::duration<qsr::f32, std::micro>(duration).count();
    case ::Time::Nanos:
        return std::chrono::duration<qsr::f32, std::nano>(duration).count();
    }

    return 0.0;
}

enum class LogLevel {
    Info,
    Warn,
};

inline auto output(std::string_view message, ::LogLevel level) -> void {

    // The super ugly \x1b stuff colors the text in the terminal output
    switch (level) {
    case ::LogLevel::Info:
        std::cout << "[\x1b[32mINFO\x1b[0m] " << message << '\n';
        break;
    case ::LogLevel::Warn:
        std::cout << "[\x1b[33mWARN\x1b[0m] " << message << '\n';
        break;
    }
}

}; // namespace
