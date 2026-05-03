#pragma once

// add header files
#include "pipeline.hpp"
#include "window_handling.hpp"
#include <glm/glm.hpp>

namespace miniEngine {
class CameraLogic {
public:
  void update(WindowHandling &window);
  CameraPushConstants getCameraPushConstants();

private:
  float radius = 10.0f;
  float theta = 3.1f / 4.0f;             // horizontal
  float phi = 3.1f / 4.0f;               // polar
  glm::vec3 center = {0.0f, 0.0f, 0.0f}; // camera pointing at
};
} // namespace miniEngine