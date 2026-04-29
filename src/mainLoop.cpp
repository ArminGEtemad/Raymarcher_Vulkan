#include "mainLoop.hpp"
#include <GLFW/glfw3.h>

namespace miniEngine {
void makeApp::run() {
  while (!createWindow.shouldClose()) {
    glfwPollEvents();
  }
}
} // namespace miniEngine