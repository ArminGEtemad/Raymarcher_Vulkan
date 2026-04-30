// add header files
#include "window_handling.hpp"

// add libraries
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <string>

namespace miniEngine {
WindowHandling::WindowHandling(int w, int h, std::string name)
    : width{w}, height{h}, windowName{name} {
  initWindow();
}

WindowHandling::~WindowHandling() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

void WindowHandling::initWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  // TODO: making the window resizable "not the priority now"
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window =
      glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);

  // runtime error if failed
  if (!window) {
    throw std::runtime_error("Failed to create GLFW window");
  }
}
void WindowHandling::createWindowSurface(VkInstance instance,
                                         VkSurfaceKHR *surface) {
  if (glfwCreateWindowSurface(instance, window, nullptr, surface) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create a surface");
  }
}
}; // namespace miniEngine