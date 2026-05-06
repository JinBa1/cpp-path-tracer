# Minimum version and project name
cmake_minimum_required(VERSION 3.10)
project(ray_tracer)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Define source files
file(GLOB SOURCES "src/*.cpp")

# Add the executable
add_executable(ray_tracer ${SOURCES})

# Add optimization flag for ray_tracer target
target_compile_options(ray_tracer PRIVATE -O1)

# Optional sanitizer flags (ASAN + UBSAN combined)
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(ENABLE_TESTING "Enable CTest" OFF)

if(ENABLE_ASAN)
    target_compile_options(ray_tracer PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(ray_tracer PRIVATE -fsanitize=address)
endif()

if(ENABLE_UBSAN)
    target_compile_options(ray_tracer PRIVATE -fsanitize=undefined)
    target_link_options(ray_tracer PRIVATE -fsanitize=undefined)
endif()

if(ENABLE_TESTING)
    enable_testing()
endif()