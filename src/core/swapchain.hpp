#pragma once

// add header files
#include "setup.hpp"
#include "window_handling.hpp"
#include <vulkan/vulkan.h>

// add libraries
#include <vector>

namespace miniEngine {
class SwapChainEngine {
public:
  SwapChainEngine(WindowHandling &window, SetupDevice &device);
  ~SwapChainEngine();

  SwapChainEngine(const SwapChainEngine &) = delete;
  SwapChainEngine &operator=(const SwapChainEngine &) = delete;

private:
  // initializations
  WindowHandling &window;
  SetupDevice &device;
  VkSwapchainKHR swapChain;
  std::vector<VkImage> swapChainImages;
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;

  // functions
  void init();
  void createSwapChain();
  // helper functions
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
};
} // namespace miniEngine