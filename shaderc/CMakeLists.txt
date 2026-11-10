add_executable(qsr-shaderc
  src/main.cc
  src/compile.cc 
  src/output.cc
)

target_include_directories(qsr-shaderc PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/inc"
)

target_compile_features(qsr-shaderc PRIVATE
  cxx_std_26
)

set_target_properties(qsr-shaderc PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  CXX_EXTENSIONS OFF
)

# Find slang SDK
find_package(slang CONFIG QUIET)

if(TARGET slang::slang)
    set(QSR_SLANG_TARGET slang::slang)
else()
    # Fallback for prebuilt SDK releases without CMake package files.
    set(SLANG_ROOT "$ENV{SLANG_ROOT}" CACHE PATH
        "Path to the Slang SDK"
    )

    find_path(SLANG_INCLUDE_DIR
        NAMES slang.h
        HINTS "${SLANG_ROOT}/include"
        REQUIRED
    )

    find_library(SLANG_LIBRARY
        NAMES slang-compiler slang
        HINTS
            "${SLANG_ROOT}/lib"
            "${SLANG_ROOT}/lib64"
        REQUIRED
    )

    add_library(qsr_slang UNKNOWN IMPORTED)

    set_target_properties(qsr_slang PROPERTIES
        IMPORTED_LOCATION "${SLANG_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${SLANG_INCLUDE_DIR}"
    )

    set(QSR_SLANG_TARGET qsr_slang)
endif()

target_link_libraries(qsr-shaderc PRIVATE 
  ${QSR_SLANG_TARGET}
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(qsr-shaderc PRIVATE
    -Wall
    -Wextra
    -Wpedantic
  )
endif()

include(GNUInstallDirs)

install(TARGETS qsr-shaderc
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
