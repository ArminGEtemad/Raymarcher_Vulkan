#pragma once
#define GLFW_INCLUDE_VULKAN

// add libraries
#include <GLFW/glfw3.h>
#include <string>
#include <vulkan/vulkan.h>

namespace miniEngine {
class WindowHandling {
public:
  WindowHandling(int w, int h, std::string name);
  ~WindowHandling();

  // cleaning up
  WindowHandling(const WindowHandling &) = delete;
  WindowHandling &operator=(const WindowHandling &) = delete;

  bool shouldClose() { return glfwWindowShouldClose(window); }
  void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
  void getFramebufferSize(int *width, int *height);

private:
  // window sizes
  const int width;
  const int height;

  GLFWwindow *window;

  std::string windowName;

  void initWindow();
};
}; // namespace miniEngine