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
  std::vector<VkImageView> swapChainImageViews;

  // functions
  void init();
  void createSwapChain();
  void createImageViews();

  // helper functions
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
};
} // namespace miniEngine