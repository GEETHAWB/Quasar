#pragma once

#include <string_view>

enum class LogLevel {
    Info,
    Success,
    Warning,
    Error,
    Debug,
};

class Output final {
public:
    static auto log(std::string_view msg, LogLevel level = LogLevel::Info) -> void;
    static auto section(std::string_view msg) -> void;
    static auto summary(std::size_t compiled, std::size_t failed) -> void;
    static auto compilation(std::string_view file) -> void;
};

// Can't not have swag when printing to the terminal
constexpr std::string_view reset = "\033[0m";
constexpr std::string_view cyan = "\033[36m";
constexpr std::string_view green = "\033[32m";
constexpr std::string_view yellow = "\033[33m";
constexpr std::string_view red = "\033[31m";
constexpr std::string_view gray = "\033[90m";
