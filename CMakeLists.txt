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