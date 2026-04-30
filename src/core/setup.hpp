#pragma once

// add header files
#include "window_handling.hpp"
#include <vulkan/vulkan.h>

// add libraries
#include <cstdint>
#include <optional>
#include <vector>

namespace miniEngine {

// queue family
struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;
  bool isComplete() {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

class SetupDevice {
public:
// macro for validation layer
#ifdef NDEBUG
  const bool enableValidationLayers = false; // for end project
#else
  const bool enableValidationLayers = true; // for testings
#endif

  SetupDevice(WindowHandling &window);
  ~SetupDevice();

  SetupDevice(const SetupDevice &) = delete;
  SetupDevice &operator=(const SetupDevice &) = delete;

private:
  // initializations
  VkInstance instance;
  VkSurfaceKHR surface;
  const std::vector<const char *> validationLayers = {
      "VK_LAYER_KHRONOS_validation"};
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceFeatures deviceFeatures;
  VkDevice device;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
  WindowHandling &window;

  // functions
  void createInstance();
  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();

  // -- helper functions --
  // device suiability
  bool isDeviceSuitable(VkPhysicalDevice device);

  // queue family
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

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