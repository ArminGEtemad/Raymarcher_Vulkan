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

// Querying details of swap chain support
struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
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

  // needed for swap chain files to be accessed
  SwapChainSupportDetails getSwapChainSupport() {
    return querySwapChainSupport(physicalDevice);
  }
  VkSurfaceKHR getSurface() { return surface; }
  VkDevice getDevice() { return device; }
  QueueFamilyIndices findPhysicalQueueFamilies() {
    return findQueueFamilies(physicalDevice);
  }
  VkCommandPool getCommandPool() { return commandPool; }
  VkQueue getGraphicsQueue() { return graphicsQueue; }
  VkQueue getPresentQueue() { return presentQueue; }

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
  VkCommandPool commandPool;

  // functions
  void createInstance();
  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createCommandPool();

  // -- helper functions --
  // device suiability
  bool isDeviceSuitable(VkPhysicalDevice device);

  // queue family
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

  // swap chain support check
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

  // validation layer
  bool checkValidationLayerSupport();
  void setupDebugMessenger();
  VkDebugUtilsMessengerEXT debugMessenger;
  void populateDebugMessengerCreateInfo(
      VkDebugUtilsMessengerCreateInfoEXT &createInfo);

  // extension checkers
  std::vector<const char *> getRequiredExtensions();
  void hasInstanceExtension();
  bool checkDeviceExtensionSupport(VkPhysicalDevice device);
  const std::vector<const char *> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};
} // namespace miniEngine