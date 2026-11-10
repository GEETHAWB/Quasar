#include "compile.hh"
#include "output.hh"

#include <slang.h>

auto main() -> int {
    using namespace qsr::shader;
    try {
        Compiler compiler{"shader"};
        compiler.compile();
        return EXIT_SUCCESS;
    } catch (const std::exception& exception) {
        Output::log(exception.what(), LogLevel::Error);
        return EXIT_FAILURE;
    }
}
