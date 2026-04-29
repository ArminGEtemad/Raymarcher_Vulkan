cmake_minimum_required(VERSION 3.10)
project(Raymarcher_Vulkan)


find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

include_directories(external src src/core)

add_executable(Raymarcher
    src/main.cpp
    src/mainLoop.cpp
    src/core/setup.cpp
    external/window_handling.cpp
)

target_link_libraries(Raymarcher Vulkan::Vulkan glfw)