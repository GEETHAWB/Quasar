#include "output.hh"

#include <print>

namespace {
auto prefix(LogLevel level) -> std::string_view {
    switch (level) {
        case LogLevel::Info:
            return gray;
        case LogLevel::Success:
            return green;
        case LogLevel::Warning:
            return yellow;
        case LogLevel::Error:
            return red;
        case LogLevel::Debug:
            return gray;
    }
}

auto label(LogLevel level) -> std::string_view {
    switch (level) {
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Success:
            return " OK ";
        case LogLevel::Warning:
            return "WARN";
        case LogLevel::Error:
            return "FAIL";
        case LogLevel::Debug:
            return "DEBUG";
    }

    return "????";
}

} // namespace
auto Output::log(std::string_view msg, LogLevel level) -> void {
    const auto color = prefix(level);
    const auto name = label(level);

    if (level == LogLevel::Error || level == LogLevel::Warning) {
        std::println(stderr, "{}[{}] {}{}", color, name, reset, msg);
        return;
    }

    std::println("{}[{}] {}{}", color, name, reset, msg);
}

auto Output::section(std::string_view msg) -> void {
    std::println("\n{}====== {} ======{}", cyan, msg, reset);
}

auto Output::compilation(std::string_view file) -> void {
    log(file, LogLevel::Info);
}

auto Output::summary(std::size_t compiled, std::size_t failed) -> void {
    section("Compilation Summary");
    log(std::format("Compiled: {}", compiled), LogLevel::Success);
    log(std::format("Failed: {}", failed), failed > 0 ? LogLevel::Error : LogLevel::Success);
}
