#include "compile.hh"

#include "output.hh"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <slang-com-ptr.h>
#include <slang.h>
#include <stdexcept>
#include <vector>

namespace fs = std::filesystem;

namespace {

auto checkResult(SlangResult result, std::string_view operation) -> void {
    if (SLANG_FAILED(result))
        throw std::runtime_error{"Slang operation failed: " + std::string{operation}};
}

auto printDiagnostics(slang::IBlob* diagnostics) -> void {
    if (!diagnostics || diagnostics->getBufferSize() == 0)
        return;

    const auto* data = static_cast<const char*>(diagnostics->getBufferPointer());

    std::string_view message{data, diagnostics->getBufferSize()};

    while (!message.empty() && message.back() == '\0') {
        message.remove_suffix(1);
    }

    if (!message.empty())
        Output::log(message, LogLevel::Warning);
}

auto findShaders(const fs::path& root, const fs::path& output) -> std::vector<fs::path> {
    std::vector<fs::path> shaders;

    for (fs::recursive_directory_iterator dir{root}, end; dir != end; ++dir) {
        if (dir->is_directory()) {
            if (dir->path().lexically_normal() == output.lexically_normal()) {
                dir.disable_recursion_pending();
            }
            continue;
        }
        if (!dir->is_regular_file())
            continue;

        if (dir->path().extension() == ".slang")
            shaders.push_back(dir->path());
    }
    std::ranges::sort(shaders);

    return shaders;
}

auto writeBinary(const fs::path& path, slang::IBlob* binary) -> void {
    if (!binary || binary->getBufferSize() == 0)
        throw std::runtime_error{"Slang produced an empty binary"};

    fs::create_directories(path.parent_path());
    std::ofstream file{path, std::ios::binary | std::ios::trunc};

    if (!file)
        throw std::runtime_error{"Unable to create SPIR-V file: " + path.string()};

    file.write(static_cast<const char*>(binary->getBufferPointer()),
               static_cast<std::streamsize>(binary->getBufferSize()));

    if (!file)
        throw std::runtime_error{"Failed writing SPIR-V file: " + path.string()};
}

}; // namespace

namespace qsr::shader {

class Compiler::Implementation final {
public:
    explicit Implementation(fs::path shaderDirectory)
        : shaderDirectory_{fs::canonical(shaderDirectory)},
          outputDirectory_{shaderDirectory_ / "spirv"} {
        if (!fs::is_directory(shaderDirectory_))
            throw std::runtime_error{"Shader directory is not a directory"};

        initialize();
    }

    auto compile() -> void {
        const auto shaders = findShaders(this->shaderDirectory_, this->outputDirectory_);

        if (shaders.empty()) {
            Output::log("No Slang shader files found", LogLevel::Warning);
            return;
        }

        Output::log("Found " + std::to_string(shaders.size()) + " source files");

        Output::section("Shader Compilation");
        std::size_t compiled = 0;
        std::size_t failed = 0;

        for (const auto& shader : shaders) {
            try {
                if (compileModule(shader))
                    ++compiled;
            } catch (const std::exception& exception) {
                ++failed;

                Output::log(shader.string() + ": " + exception.what(), LogLevel::Error);
            }
        }

        Output::summary(compiled, failed);

        if (failed > 0)
            throw std::runtime_error{"One or more shaders failed to compile"};
    }

private:
    fs::path shaderDirectory_;
    fs::path outputDirectory_;

    Slang::ComPtr<slang::IGlobalSession> globalSession_;
    Slang::ComPtr<slang::ISession> session_;

    auto initialize() -> void {
        Output::section("Compilation Info");

        ::checkResult(slang::createGlobalSession(this->globalSession_.writeRef()),
                      "createGlobalSession");

        slang::TargetDesc target{};
        target.format = SLANG_SPIRV;
        target.profile = this->globalSession_->findProfile("spirv_1_5");
        target.flags = SLANG_TARGET_FLAG_GENERATE_WHOLE_PROGRAM;
        if (target.profile == SLANG_PROFILE_UNKNOWN)
            throw std::runtime_error{"SPIR-V 1.5 profile is unavailable"};

        const auto searchPath = this->shaderDirectory_.string();
        const char* const searchPaths[] = {searchPath.c_str()};
        const std::array options{slang::CompilerOptionEntry{
            slang::CompilerOptionName::EmitSpirvDirectly,
            {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}}};

        slang::SessionDesc description{};
        description.targets = &target;
        description.targetCount = 1;
        description.searchPaths = searchPaths;
        description.searchPathCount = 1;
        description.compilerOptionEntries = options.data();
        description.compilerOptionEntryCount = static_cast<SlangInt>(options.size());

        ::checkResult(this->globalSession_->createSession(description, this->session_.writeRef()),
                      "createSession");
        Output::log("Initialized Slang with SPIR-V 1.5", LogLevel::Success);
    }

    auto compileModule(const fs::path& sourcePath) -> bool {
        auto relativePath = sourcePath.lexically_relative(this->shaderDirectory_);
        Output::compilation(relativePath.string());
        relativePath.replace_extension();
        const auto moduleName = relativePath.generic_string();

        const auto sourceName = sourcePath.string();
        std::ifstream file{sourcePath, std::ios::binary};
        if (!file)
            throw std::runtime_error{"Unable to read source file"};

        const std::string source{std::istreambuf_iterator<char>{file},
                                 std::istreambuf_iterator<char>{}};
        Slang::ComPtr<slang::IBlob> diagnostics;

        Slang::ComPtr<slang::IModule> module{this->session_->loadModuleFromSourceString(
            moduleName.c_str(), sourceName.c_str(), source.c_str(), diagnostics.writeRef())};

        ::printDiagnostics(diagnostics.get());
        if (!module)
            throw std::runtime_error{"Failed to load Slang module"};
        const auto entryPointCount = module->getDefinedEntryPointCount();
        if (entryPointCount == 0) {
            Output::log("No shader entry points; skipping module", LogLevel::Warning);
            return false;
        }

        Slang::ComPtr<slang::IComponentType> linkedModule;
        diagnostics = nullptr;
        const auto linkResult = module->link(linkedModule.writeRef(), diagnostics.writeRef());

        ::printDiagnostics(diagnostics.get());
        ::checkResult(linkResult, "module linking");

        Slang::ComPtr<slang::IBlob> binary;
        diagnostics = nullptr;
        const auto compileResult =
            linkedModule->getTargetCode(0, binary.writeRef(), diagnostics.writeRef());
        ::printDiagnostics(diagnostics.get());
        ::checkResult(compileResult, "SPIR-V generation");

        auto outputPath = this->outputDirectory_ / relativePath;

        outputPath.replace_extension(".spv");
        ::writeBinary(outputPath, binary.get());
        Output::log("Generated " + outputPath.string() + " (" + std::to_string(entryPointCount) +
                        " entry points)",
                    LogLevel::Success);

        return true;
    }
};

Compiler::Compiler(fs::path shaderDirectory)
    : implementation_{std::make_unique<Implementation>(std::move(shaderDirectory))} {
}

Compiler::~Compiler() = default;
auto Compiler::compile() -> void {
    this->implementation_->compile();
}

}; // namespace qsr::shader
