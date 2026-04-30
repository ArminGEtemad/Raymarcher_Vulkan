#pragma once

// add header files
#include "window_handling.hpp"
#include <vulkan/vulkan.h>

// add libraries
#include <vector>

namespace miniEngine {
class SetupDevice {
public:
// macro for validation layer
#ifdef NDEBUG
  const bool enableValidationLayers = false; // for end project
#else
  const bool enableValidationLayers = true; // for testings
#endif

  SetupDevice();
  ~SetupDevice();

  SetupDevice(const SetupDevice &) = delete;
  SetupDevice &operator=(const SetupDevice &) = delete;

private:
  // initializations
  VkInstance instance;
  void createInstance();

  const std::vector<const char *> validationLayers = {
      "VK_LAYER_KHRONOS_validation"};

  // validation layer
  bool checkValidationLayerSupport();
  void setupDebugMessenger();
  VkDebugUtilsMessengerEXT debugMessenger;
  void populateDebugMessengerCreateInfo(
      VkDebugUtilsMessengerCreateInfoEXT &createInfo);

  // extension checkers
  std::vector<const char *> getRequiredExtensions();
  void hasInstanceExtension();
};
} // namespace miniEngine