#include "cameraLogic.hpp"
#include "pipeline.hpp"
#include "window_handling.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <glm/fwd.hpp>

namespace miniEngine {
void CameraLogic::update(WindowHandling &window) {
  GLFWwindow *nativeWin = window.getGLFWwindow();
  float sensitivity = 0.01f;

  // zoom
  if (glfwGetKey(nativeWin, GLFW_KEY_1) == GLFW_PRESS)
    radius -= sensitivity;
  if (glfwGetKey(nativeWin, GLFW_KEY_2) == GLFW_PRESS)
    radius += sensitivity;
  // orbit
  if (glfwGetKey(nativeWin, GLFW_KEY_A) == GLFW_PRESS)
    theta -= sensitivity;
  if (glfwGetKey(nativeWin, GLFW_KEY_D) == GLFW_PRESS)
    theta += sensitivity;

  if (glfwGetKey(nativeWin, GLFW_KEY_W) == GLFW_PRESS)
    phi -= sensitivity;
  if (glfwGetKey(nativeWin, GLFW_KEY_S) == GLFW_PRESS)
    phi += sensitivity;

  phi = std::clamp(phi, 0.1f, 3.14f - 0.1f);

  // move the camera target
  if (glfwGetKey(nativeWin, GLFW_KEY_UP) == GLFW_PRESS) {
    // x direction
    if (glfwGetKey(nativeWin, GLFW_KEY_X) == GLFW_PRESS)
      cameraTarget[0] += sensitivity;
    // y direction
    if (glfwGetKey(nativeWin, GLFW_KEY_Y) == GLFW_PRESS)
      cameraTarget[1] += sensitivity;
    // z direction
    if (glfwGetKey(nativeWin, GLFW_KEY_Z) == GLFW_PRESS)
      cameraTarget[2] += sensitivity;
  }

  if (glfwGetKey(nativeWin, GLFW_KEY_DOWN) == GLFW_PRESS) {
    // x direction
    if (glfwGetKey(nativeWin, GLFW_KEY_X) == GLFW_PRESS)
      cameraTarget[0] -= sensitivity;
    // y direction
    if (glfwGetKey(nativeWin, GLFW_KEY_Y) == GLFW_PRESS)
      cameraTarget[1] -= sensitivity;
    // z direction
    if (glfwGetKey(nativeWin, GLFW_KEY_Z) == GLFW_PRESS)
      cameraTarget[2] -= sensitivity;
  }

  // snap back at the origin
  if (glfwGetKey(nativeWin, GLFW_KEY_O) == GLFW_PRESS)
    cameraTarget = glm::vec3(0.0, 0.0, 0.0);
}
CameraPushConstants CameraLogic::getCameraPushConstants() {
  // coordinate transformation
  CameraPushConstants cameraData;
  cameraData.position.x = radius * sin(phi) * cos(theta);
  cameraData.position.y = radius * cos(phi);
  cameraData.position.z = radius * sin(phi) * sin(theta);

  // if camera target moves
  cameraData.position += cameraTarget;

  cameraData.target = cameraTarget;
  return cameraData;
}
} // namespace miniEngine