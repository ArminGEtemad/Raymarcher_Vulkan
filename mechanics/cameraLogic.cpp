#include "cameraLogic.hpp"
#include "pipeline.hpp"
#include "window_handling.hpp"
#include <algorithm>

namespace miniEngine {
void CameraLogic::update(WindowHandling &window) {
  GLFWwindow *nativeWin = window.getGLFWwindow();
  float sensitivity = 0.01f;

  if (glfwGetKey(nativeWin, GLFW_KEY_A) == GLFW_PRESS)
    theta -= sensitivity;
  if (glfwGetKey(nativeWin, GLFW_KEY_D) == GLFW_PRESS)
    theta += sensitivity;

  if (glfwGetKey(nativeWin, GLFW_KEY_W) == GLFW_PRESS)
    phi -= sensitivity;
  if (glfwGetKey(nativeWin, GLFW_KEY_S) == GLFW_PRESS)
    phi += sensitivity;

  phi = std::clamp(phi, 0.1f, 3.14f - 0.1f);
}
CameraPushConstants CameraLogic::getCameraPushConstants() {
  CameraPushConstants cameraPosition;
  cameraPosition.position.x = radius * sin(phi) * cos(theta);
  cameraPosition.position.y = radius * cos(phi);
  cameraPosition.position.z = radius * sin(phi) * sin(theta);
  return cameraPosition;
}
} // namespace miniEngine