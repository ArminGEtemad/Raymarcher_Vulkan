#include "mainLoop.hpp"
#include <GLFW/glfw3.h>

namespace miniEngine {

makeApp::makeApp() {}
makeApp::~makeApp() {}

void makeApp::run() {
  while (!createWindow.shouldClose()) {
    glfwPollEvents();
  }
}
} // namespace miniEngine