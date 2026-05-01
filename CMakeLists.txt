cmake_minimum_required(VERSION 3.10)
project(Raymarcher_Vulkan)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

include_directories(external src src/core)

add_executable(Raymarcher
    src/main.cpp
    src/mainLoop.cpp
    src/core/setup.cpp
    src/core/swapchain.cpp
    external/window_handling.cpp
)

target_link_libraries(Raymarcher Vulkan::Vulkan glfw)