#pragma once

// add header files
#include "window_handling.hpp"
#include <vulkan/vulkan.h>

namespace miniEngine {
class SetupDevice {
public:
  SetupDevice();
  ~SetupDevice();

  SetupDevice(const SetupDevice &) = delete;
  SetupDevice &operator=(const SetupDevice &) = delete;

private:
  VkInstance instance;
  void createInstance();

  // extension checkers
  void hasInstanceExtension();
};
} // namespace miniEngine