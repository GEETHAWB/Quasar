cmake_minimum_required(VERSION 3.24)

project(Quasar
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
include(FetchContent)

find_package(Vulkan REQUIRED)
find_package(SDL2 REQUIRED)

set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.3
    GIT_SHALLOW    TRUE
)
 
FetchContent_Declare(
    VulkanMemoryAllocator
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG        v3.4.0
    GIT_SHALLOW    TRUE
)
 
FetchContent_MakeAvailable(glm VulkanMemoryAllocator)

set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
file(GLOB_RECURSE SOURCES 
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc"    
)

option(DEBUG_MODE "Enable additional debug output" OFF)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(Quasar PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/inc"
)

if (DEBUG_MODE)
    target_compile_definitions(Quasar PRIVATE DEBUG_MODE=1)
endif()

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_26)
 
target_link_libraries(${PROJECT_NAME} PRIVATE
    Vulkan::Headers
    glm::glm
    GPUOpen::VulkanMemoryAllocator
    SDL2::SDL2
    SDL2::SDL2main
)
 
# Use Vulkan's dynamic dipatch instead of Vulkan function prototypes
target_compile_definitions(${PROJECT_NAME} PRIVATE
    VK_NO_PROTOTYPES
)

target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
    
# Debug-only options
target_compile_options(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:-Wconversion>"
    "$<$<CONFIG:Debug>:-Wshadow>"
    "$<$<CONFIG:Debug>:-Og>"
    "$<$<CONFIG:Debug>:-g3>"
)

# Release-only options
target_compile_options(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Release>:-O3>"
)

set(QUASAR_FORMAT_SOURCES)
foreach(source_file IN LISTS QUASAR_SOURCES)
    list(APPEND QUASAR_FORMAT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${source_file}")
endforeach()

add_custom_target(format
    COMMAND ${CLANG_FORMAT_EXECUTABLE} -i --style=file ${QUASAR_FORMAT_SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Formatting Quasar sources"
    VERBATIM
)

add_custom_target(format-check
    COMMAND ${CLANG_FORMAT_EXECUTABLE} --dry-run --Werror --style=file ${QUASAR_FORMAT_SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Checking Quasar source formatting"
    VERBATIM
)
