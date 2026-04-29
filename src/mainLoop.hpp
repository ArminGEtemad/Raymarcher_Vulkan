#pragma once

// add header files
#include "setup.hpp"
#include "window_handling.hpp"

namespace miniEngine {
class makeApp {
public:
  static constexpr int WIDTH = 1000;
  static constexpr int HEIGHT = 800;

  void run();

private:
  WindowHandling createWindow{WIDTH, HEIGHT, "Raymarcher Grapher"};
  SetupDevice device{};
};
} // namespace miniEngine
