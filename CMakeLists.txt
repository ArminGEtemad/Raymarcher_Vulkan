cmake_minimum_required(VERSION 3.10)
project(Raymarcher_Vulkan)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

include_directories(external src src/core mechanics)

add_executable(Raymarcher
    src/main.cpp
    src/mainLoop.cpp
    src/core/setup.cpp
    src/core/swapchain.cpp
    src/core/pipeline.cpp
    external/window_handling.cpp
    mechanics/cameraLogic.cpp
)

find_program(GLSLANG_VALIDATOR NAMES glslangValidator)

if(NOT GLSLANG_VALIDATOR)
    message(FATAL_ERROR "glslangValidator not found! Is the Vulkan SDK installed?")
endif()

set(SHADER_SOURCES
    shaders/fullscreen.vert
    shaders/raymarch.frag
)

foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    set(SPIRV_OUT "${CMAKE_BINARY_DIR}/shaders/${FILE_NAME}.spv")
    
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/shaders")

    add_custom_command(
        OUTPUT ${SPIRV_OUT}
        COMMAND ${GLSLANG_VALIDATOR} -V ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER} -o ${SPIRV_OUT}
        DEPENDS ${SHADER}
        COMMENT "Compiling shader: ${SHADER}"
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV_OUT})
endforeach()

add_custom_target(CompileShaders ALL DEPENDS ${SPIRV_BINARY_FILES})
add_dependencies(Raymarcher CompileShaders)

target_link_libraries(Raymarcher Vulkan::Vulkan glfw)